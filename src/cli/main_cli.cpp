#include "catalog/catalog.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <system_error>
#include <filesystem>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {

// Default CSV file name bundled with the project. Keeping it constexpr just means
// the value never changes and can be reused anywhere we need the default name.
constexpr char kDefaultCourseCSVFile[] = "data/CS 300 ABCU_Advising_Program_Input.csv";

// Tracks whether any course data has been loaded in this session.
bool loadedData = false;
Catalog courseCatalog;
LoadResult lastLoadResult;
std::string currentCatalogPath;
std::string advisorGuiExecutable = "advisor_gui";  // Falls back to PATH lookup when we cannot resolve a build-local binary.

enum class TextStyle {
    Reset,
    MenuBorder,
    MenuTitle,
    MenuNumber,
    MenuText,
    Prompt,
    Success,
    Warning,
    Error,
    Info
};

// Palette bundles the color codes for each UI element.
struct ColorPalette {
    const char* border = "";
    const char* title = "";
    const char* number = "";
    const char* text = "";
    const char* prompt = "";
    const char* success = "";
    const char* warning = "";
    const char* error = "";
    const char* info = "";
    const char* reset = "";
};

// Allows the frame characters to swap between ASCII/Unicode/no-frame.
struct FrameStyle {
    std::string topLeft;
    std::string topRight;
    std::string bottomLeft;
    std::string bottomRight;
    std::string horizontal;
    std::string vertical;
};

// Track color support once (we honour NO_COLOR so users can force plain output).
bool colorsEnabled() {
    static const bool enabled = (std::getenv("NO_COLOR") == nullptr);
    return enabled;
}

// Lowercases a string so theme/frame environment values can be matched easily.
std::string toLowerCopy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

// Convenience helper: fetch an environment variable and return a lowercase copy.
std::string envLower(const char* name) {
    const char* raw = std::getenv(name);
    if (!raw) {
        return "";
    }
    return toLowerCopy(std::string(raw));
}

// Picks the color palette once at startup. COURSE_ADVISOR_THEME accepts
// "dark", "light", or "plain" (plain keeps colours off even if NO_COLOR is unset).
const ColorPalette& activePalette() {
    static const ColorPalette darkPalette{
        "\x1b[95m",   // border
        "\x1b[97;1m", // title
        "\x1b[93;1m", // number
        "\x1b[97m",   // text
        "\x1b[96;1m", // prompt
        "\x1b[92m",   // success
        "\x1b[93m",   // warning
        "\x1b[91m",   // error
        "\x1b[94m",   // info
        "\x1b[0m"      // reset
    };

    static const ColorPalette lightPalette{
        "\x1b[35m",   // border (slightly darker magenta)
        "\x1b[30;1m", // title (bold black)
        "\x1b[34;1m", // number (bold blue)
        "\x1b[30m",   // text (black)
        "\x1b[36;1m", // prompt (bright cyan)
        "\x1b[32m",   // success (green)
        "\x1b[33m",   // warning (gold)
        "\x1b[31m",   // error (red)
        "\x1b[35m",   // info (magenta)
        "\x1b[0m"      // reset
    };

    static const ColorPalette plainPalette{};  // Every entry is "".

    static const ColorPalette* palette = []() {
        if (!colorsEnabled()) {
            return &plainPalette;
        }

        const std::string choice = envLower("COURSE_ADVISOR_THEME");
        if (choice == "light") {
            return &lightPalette;
        }
        if (choice == "plain" || choice == "none" || choice == "off") {
            return &plainPalette;
        }
        // Default to the high-contrast dark palette. Users can force it with COURSE_ADVISOR_THEME=dark.
        return &darkPalette;
    }();

    return *palette;
}

// Allows swapping the border style via COURSE_ADVISOR_FRAME (ascii/unicode/none).
const FrameStyle& activeFrame() {
    static const FrameStyle asciiFrame{ "+", "+", "+", "+", "-", "|" };
    static const FrameStyle unicodeFrame{ "╔", "╗", "╚", "╝", "═", "║" };
    static const FrameStyle noFrame{ "", "", "", "", "", "" };

    static const FrameStyle* frame = []() {
        const std::string choice = envLower("COURSE_ADVISOR_FRAME");
        if (choice == "unicode") {
            return &unicodeFrame;
        }
        if (choice == "none" || choice == "off") {
            return &noFrame;
        }
        // ASCII borders are the safest default across terminals.
        return &asciiFrame;
    }();

    return *frame;
}

const char* ansi(TextStyle style) {
    const ColorPalette& palette = activePalette();

    switch (style) {
        case TextStyle::MenuBorder:
            return palette.border;
        case TextStyle::MenuTitle:
            return palette.title;
        case TextStyle::MenuNumber:
            return palette.number;
        case TextStyle::MenuText:
            return palette.text;
        case TextStyle::Prompt:
            return palette.prompt;
        case TextStyle::Success:
            return palette.success;
        case TextStyle::Warning:
            return palette.warning;
        case TextStyle::Error:
            return palette.error;
        case TextStyle::Info:
            return palette.info;
        case TextStyle::Reset:
        default:
            return palette.reset;
    }
}

// Keeps both raw and colored versions of a line so we can pad accurately while styling output.
struct StyledLine {
    std::string plain;
    std::string colored;
};

// Helper used when drawing frame horizontals.
std::string repeatPattern(const std::string& pattern, std::size_t count) {
    std::string result;
    result.reserve(pattern.size() * count);
    for (std::size_t i = 0; i < count; ++i) {
        result += pattern;
    }
    return result;
}

// Prints a block of lines with the active frame style, keeping spacing consistent.
void printFramedLines(const std::vector<StyledLine>& lines) {
    const FrameStyle& frame = activeFrame();
    const char* borderColor = ansi(TextStyle::MenuBorder);
    const char* resetColor = ansi(TextStyle::Reset);

    std::size_t maxWidth = 0;
    for (const auto& line : lines) {
        maxWidth = std::max(maxWidth, line.plain.size());
    }
    const std::size_t innerWidth = maxWidth + 2;  // One space of padding on each side.

    if (frame.vertical.empty()) {
        for (const auto& line : lines) {
            if (!line.colored.empty()) {
                std::cout << line.colored << resetColor;
            }
            std::cout << '\n';
        }
        return;
    }

    const std::string top = frame.topLeft + repeatPattern(frame.horizontal, innerWidth) + frame.topRight;
    const std::string bottom = frame.bottomLeft + repeatPattern(frame.horizontal, innerWidth) + frame.bottomRight;

    std::cout << borderColor << top << resetColor << '\n';
    for (const auto& line : lines) {
        const std::size_t padding = maxWidth - line.plain.size();
        std::cout << borderColor << frame.vertical << resetColor << ' ';
        if (!line.colored.empty()) {
            std::cout << line.colored << resetColor;
        }
        std::cout << std::string(padding, ' ') << ' '
                  << borderColor << frame.vertical << resetColor << '\n';
    }
    std::cout << borderColor << bottom << resetColor << '\n';
}

/**
 * Trims spaces, tabs, and newline-style characters off both ends of a string.
 * Used while we're going through CSV input and user prompts.
 */
std::string trim(const std::string& text) {
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

// Converts the provided string to uppercase.
std::string toUpper(std::string value) {
    for (char& ch : value) {
        ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    }
    return value;
}

/**
 * Makes sure a course ID starts with letters and ends with digits (think "CSCI200").
 * Anything that breaks that pattern is rejected.
 */
bool isCourseIdValid(const std::string& courseId) {
    if (courseId.empty()) {
        return false;
    }

    bool hasLetter = false;
    bool hasDigit = false;

    for (char ch : courseId) {
        if (std::isalpha(static_cast<unsigned char>(ch))) {
            if (hasDigit) {
                return false;
            }
            hasLetter = true;
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(ch))) {
            hasDigit = true;
            continue;
        }

        return false;
    }

    return hasLetter && hasDigit;
}

// Removes a trailing comma from user input and trims in case it was pasted from CSV output.
static void removeTrailingComma(std::string& text) {
    if (!text.empty() && text.back() == ',') {
        text.pop_back();
        text = trim(text);
    }
}

struct NormalizedCourseId {
    std::string id;
    bool wasTrimmed = false;
};

/**
 * Cleans up what the user typed for a course lookup. We keep the leading letters,
 * then any digits, drop the rest, and tell the caller if we had to tweak it.
 */
std::optional<NormalizedCourseId> normalizeCourseIdInput(std::string input) {
    // Trim whitespace, convert to uppercase, and handle any stray commas first.
    input = toUpper(trim(input));
    removeTrailingComma(input);
    if (input.empty()) {
        return std::nullopt;
    }

    std::string parsedId;
    parsedId.reserve(input.size());  // Avoid reallocations while we build the ID.

    bool hasDigit = false;  // Track when we switch from letters to digits.
    for (char ch : input) {
        if (std::isalpha(static_cast<unsigned char>(ch))) {
            if (hasDigit) {
                break;
            }
            parsedId.push_back(ch);
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(ch))) {
            hasDigit = true;
            parsedId.push_back(ch);
            continue;
        }

        // Unexpected characters end the parse.
        break;
    }

    if (!isCourseIdValid(parsedId)) {
        return std::nullopt;
    }

    NormalizedCourseId result;
    result.id = std::move(parsedId);
    result.wasTrimmed = (result.id != input);  // Flag when we had to trim or fix input.
    return result;
}

/**
 * Prints the outcome from the most recent catalog load so the CLI and GUI can stay in sync.
 * Warnings are surfaced individually to stay consistent with the original behaviour.
 */
void reportLoadMessages(const LoadResult& loadResult) {
    for (const auto& warning : loadResult.warnings) {
        std::cout << ansi(TextStyle::Warning) << warning << '\n' << ansi(TextStyle::Reset);
    }

    if (loadResult.missingPrerequisites.empty()) {
        std::cout << ansi(TextStyle::Success)
                  << "All prerequisites found in the loaded catalog.\n"
                  << ansi(TextStyle::Reset);
    } else {
        for (const auto& missing : loadResult.missingPrerequisites) {
            std::cout << ansi(TextStyle::Warning)
                      << "Prerequisite missing from catalog: " << missing << '\n'
                      << ansi(TextStyle::Reset);
        }
    }
}

/**
 * Prints the complete Computer Science program catalog (including math support
 * courses) in alphanumeric order using the cached list built during load.
 */
void printAllCourses() {
    const std::vector<std::string> ids = courseCatalog.ids();
    if (ids.empty()) {
        std::cout << ansi(TextStyle::Warning)
                  << "No courses available to display.\n"
                  << ansi(TextStyle::Reset);
        return;
    }

    std::vector<StyledLine> lines;
    lines.push_back({
        "Course List",
        std::string(ansi(TextStyle::MenuTitle)) + "Course List"
    });
    lines.push_back({"", ""});
    for (const auto& id : ids) {
        const Course* course = courseCatalog.get(id);
        if (!course) {
            continue;
        }
        std::string plain = course->courseNumber + ", " + course->courseName;
        lines.push_back({
            plain,
            std::string(ansi(TextStyle::MenuText)) + plain
        });
    }

    std::cout << '\n';
    printFramedLines(lines);
    std::cout << '\n';
}

/**
 * Prints one course along with the full names of its prerequisites and calls out
 * any missing prerequisite entries so they are easy to spot.
 */
void printCourseDetails(const Course& courseDetails) {
    std::cout << ansi(TextStyle::MenuTitle) << courseDetails.courseNumber
              << ansi(TextStyle::Reset) << ", " << courseDetails.courseName << '\n';

    if (courseDetails.prerequisites.empty()) {
        std::cout << ansi(TextStyle::Info) << "Prerequisites: none\n"
                  << ansi(TextStyle::Reset);
        return;
    }

    std::cout << ansi(TextStyle::MenuBorder) << "Prerequisites:\n"
              << ansi(TextStyle::Reset);
    for (const auto& prereqId : courseDetails.prerequisites) {
        std::cout << ansi(TextStyle::MenuNumber) << "  " << prereqId
                  << ansi(TextStyle::Reset);
        const Course* prereq = courseCatalog.get(prereqId);
        if (prereq) {
            std::cout << " - " << prereq->courseName;
        } else {
            std::cout << " - " << ansi(TextStyle::Warning)
                      << "(missing from catalog)" << ansi(TextStyle::Reset);
        }
        std::cout << '\n';
    }
}

/**
 * Reads the CSV file, cleans up the IDs, checks prerequisites, and loads the
 * results using the shared catalog core before caching the sorted lists.
 */
bool loadCoursesFromFile(const std::string& fileName) {
    lastLoadResult = courseCatalog.load(fileName);
    loadedData = lastLoadResult.ok;

    if (!loadedData) {
        for (const auto& warning : lastLoadResult.warnings) {
            std::cout << ansi(TextStyle::Error) << warning << '\n'
                      << ansi(TextStyle::Reset);
        }
        std::cout << ansi(TextStyle::Warning)
                  << "No courses were loaded from " << fileName << '\n'
                  << ansi(TextStyle::Reset);
        return false;
    }

    currentCatalogPath = lastLoadResult.path;
    std::cout << ansi(TextStyle::Success) << "Loaded "
              << lastLoadResult.courses << " courses from "
              << currentCatalogPath << '\n' << ansi(TextStyle::Reset);
    reportLoadMessages(lastLoadResult);
    std::cout << ansi(TextStyle::Success) << "Courses have been loaded!\n"
              << ansi(TextStyle::Reset);

    return true;
}

/**
 * Prompts for a course ID, cleans it up, and prints the matching course details
 * (including prerequisite titles) when present in the course directory.
 */
void handleCourseLookup() {
    std::cout << ansi(TextStyle::Prompt) << "Enter the course number: "
              << ansi(TextStyle::Reset);
    std::string input;
    if (!std::getline(std::cin, input)) {
        return;
    }

    auto sanitizedId = normalizeCourseIdInput(input);  // Clean up the user input.
    if (!sanitizedId) {
        std::cout << ansi(TextStyle::Error)
                  << "Course number must start with letters and end with digits.\n"
                  << ansi(TextStyle::Reset);
        return;
    }

    if (sanitizedId->wasTrimmed) {
        // Let the user know which course number we ended up using after cleanup.
        std::cout << ansi(TextStyle::Info) << "Searching for course: "
                  << sanitizedId->id << '\n' << ansi(TextStyle::Reset);
    }

    const Course* locatedCourse = courseCatalog.get(sanitizedId->id);
    if (!locatedCourse) {
        std::cout << ansi(TextStyle::Error) << "Course not found: "
                  << sanitizedId->id << '\n' << ansi(TextStyle::Reset);
        return;
    }

    printCourseDetails(*locatedCourse);
}

// Pauses so the user can read results before the menu redraws.
void waitForEnter() {
    if (!std::cin.good() && !std::cin.eof()) {
        std::cin.clear();
    }
    std::cout << ansi(TextStyle::Prompt) << "Press Enter to continue..." << ansi(TextStyle::Reset) << std::flush;
    std::string pause;
    std::getline(std::cin, pause);
}

/**
 * Helper that launches the Qt dashboard binary. When a catalog is already loaded
 * we pass the resolved path so the GUI can hydrate immediately.
 */
void launchDashboard() {
    std::string command = "\"";  // Quote the executable in case the build path has spaces.
    command += advisorGuiExecutable;
    command += "\"";
    if (!currentCatalogPath.empty()) {
        command += " \"";
        command += currentCatalogPath;
        command += '"';
    }

    const int exitCode = std::system(command.c_str());
    if (exitCode != 0) {
        std::cout << ansi(TextStyle::Warning)
                  << "Dashboard exited with code " << exitCode << '\n'
                  << ansi(TextStyle::Reset);
    }
}

/**
 * Main loop for the menu: handles load, print, lookup, dashboard launch, and exit.
 * Actions that depend on catalog data are gated behind a successful load.
 */
void runMenu() {
    while (true) {
        const char* numberColor = ansi(TextStyle::MenuNumber);
        const char* textColor = ansi(TextStyle::MenuText);
        const char* promptColor = ansi(TextStyle::Prompt);
        const char* resetColor = ansi(TextStyle::Reset);

        std::vector<StyledLine> menuLines;
        menuLines.push_back({
            "Course Advisor Menu",
            std::string(ansi(TextStyle::MenuTitle)) + "Course Advisor Menu"
        });
        menuLines.push_back({"", ""});
        menuLines.push_back({
            "1. Load the courses from the file",
            std::string(numberColor) + "1. " + textColor + "Load the courses from the file"
        });
        menuLines.push_back({
            "2. Print Computer Science course list in alphanumeric order",
            std::string(numberColor) + "2. " + textColor +
            "Print Computer Science course list in alphanumeric order"
        });
        menuLines.push_back({
            "3. Find a course by the course number",
            std::string(numberColor) + "3. " + textColor +
            "Find a course by the course number"
        });
        menuLines.push_back({
            "4. Launch Qt dashboard",
            std::string(numberColor) + "4. " + textColor + "Launch Qt dashboard"
        });
        menuLines.push_back({
            "9. Exit",
            std::string(numberColor) + "9. " + textColor + "Exit"
        });

        printFramedLines(menuLines);
        std::cout << promptColor << "Enter option: " << resetColor;

        std::string choice;
        if (!std::getline(std::cin, choice)) {
            std::cout << '\n' << ansi(TextStyle::Info)
                      << "Input stream closed. Exiting.\n" << ansi(TextStyle::Reset);
            break;
        }

        choice = trim(choice);

        if (choice == "1") {
            std::cout << promptColor << "Enter file name: " << resetColor;
            std::string fileName;
            if (!std::getline(std::cin, fileName)) {
                std::cout << '\n' << ansi(TextStyle::Info)
                          << "Input stream closed. Exiting.\n" << ansi(TextStyle::Reset);
                break;
            }

            fileName = trim(fileName);
            if (!fileName.empty() && fileName.back() == ',') {
                std::cout << ansi(TextStyle::Warning)
                          << "Ignoring trailing comma in file name input.\n"
                          << ansi(TextStyle::Reset);
                removeTrailingComma(fileName);
            }
            if (fileName.empty()) {
                std::cout << ansi(TextStyle::Info) << "Using default catalog file: "
                          << kDefaultCourseCSVFile << '\n' << ansi(TextStyle::Reset);
                fileName = kDefaultCourseCSVFile;
            }

            loadCoursesFromFile(fileName);
            waitForEnter();
        } else if (choice == "2") {
            if (!loadedData) {
                std::cout << ansi(TextStyle::Warning)
                          << "Please load courses first (option 1).\n"
                          << ansi(TextStyle::Reset);
                waitForEnter();
                continue;
            }
            printAllCourses();
            waitForEnter();
        } else if (choice == "3") {
            if (!loadedData) {
                std::cout << ansi(TextStyle::Warning)
                          << "Please load courses first (option 1).\n"
                          << ansi(TextStyle::Reset);
                waitForEnter();
                continue;
            }
            handleCourseLookup();
            waitForEnter();
        } else if (choice == "4") {
            launchDashboard();
            waitForEnter();
        } else if (choice == "9") {
            std::cout << ansi(TextStyle::Success) << "Goodbye.\n" << ansi(TextStyle::Reset);
            break;
        } else {
            std::cout << ansi(TextStyle::Error)
                      << "Error, please enter option 1, 2, 3, 4, or 9.\n"
                      << ansi(TextStyle::Reset);
            waitForEnter();
        }
    }
}

}  // namespace

// Main starts the menu loop.
int main(int argc, char** argv) {
    if (argc > 0 && argv[0] != nullptr) {
        std::filesystem::path executablePath = std::filesystem::absolute(argv[0]);
        std::filesystem::path executableDir = executablePath.parent_path();
        std::filesystem::path guiPath = executableDir / "advisor_gui";  // Look beside the CLI for the GUI build.
#ifdef _WIN32
        guiPath += ".exe";
#endif
        std::error_code existsError;
        if (std::filesystem::exists(guiPath, existsError)) {
            advisorGuiExecutable = guiPath.string();
        }
    }
    runMenu();
    return 0;
}
