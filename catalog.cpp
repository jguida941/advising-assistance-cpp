#include "catalog.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <optional>
#include <set>
#include <sstream>
#include <unordered_map>

namespace {

using std::string;

// Matches the search depth used by the CLI to locate CSV files.
constexpr int kMaxParentSearchDepth = 10;

// Mirrors the trim helper in the CLI so both paths treat whitespace the same way.
string trim(const string& text) {
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == string::npos) {
        return "";
    }
    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

// Converts the provided string to uppercase to keep IDs normalized.
string toUpper(string value) {
    for (char& ch : value) {
        ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    }
    return value;
}

/**
 * Makes sure a course ID starts with letters and ends with digits (think "CSCI200").
 * Anything that breaks that pattern is rejected.
 */
bool isCourseIdValid(const string& courseId) {
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

/**
 * Looks for the course data file by name, starting in the current directory and
 * walking up the parents so the program still works when run from build folders.
 */
std::optional<std::filesystem::path> resolveCourseFilePath(const std::string& fileName) {
    namespace fs = std::filesystem;
    if (fileName.empty()) {
        return std::nullopt;
    }

    fs::path requested(fileName);
    if (requested.is_absolute()) {
        if (fs::exists(requested)) {
            return fs::absolute(requested);
        }
        return std::nullopt;
    }

    if (fs::exists(requested)) {
        return fs::absolute(requested);
    }

    fs::path searchDir = fs::current_path();
    for (int depth = 0; depth < kMaxParentSearchDepth; ++depth) {
        const fs::path candidate = searchDir / requested;
        if (fs::exists(candidate)) {
            return fs::absolute(candidate);
        }

        if (!searchDir.has_parent_path()) {
            break;
        }
        searchDir = searchDir.parent_path();
    }

    return std::nullopt;
}

}  // namespace

LoadResult Catalog::load(const std::string& fileName) {
    LoadResult result;
    if (fileName.empty()) {
        result.warnings.emplace_back("File name is empty.");
        return result;
    }

    const auto resolvedPath = resolveCourseFilePath(fileName);
    if (!resolvedPath) {
        result.warnings.emplace_back("Unable to locate file: " + fileName);
        result.path = fileName;
        return result;
    }

    result.path = resolvedPath->string();

    std::ifstream input(*resolvedPath);
    if (!input.is_open()) {
        result.warnings.emplace_back("Unable to open file: " + result.path);
        return result;
    }

    // Build up a fresh directory so we only swap the member data once the file succeeds.
    std::unordered_map<std::string, Course> loadedCourseDirectory;
    std::vector<std::string> warnings;

    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(input, line)) {
        ++lineNumber;
        line = trim(line);
        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);
        std::vector<std::string> columns;
        std::string cell;
        while (std::getline(ss, cell, ',')) {
            columns.push_back(trim(cell));
        }

        if (columns.size() < 2) {
            warnings.emplace_back("Skipping line " + std::to_string(lineNumber) +
                                  ": expected course ID and name.");
            continue;
        }

        std::string courseId = toUpper(columns[0]);
        if (!isCourseIdValid(courseId)) {
            warnings.emplace_back("Skipping line " + std::to_string(lineNumber) +
                                  ": invalid course ID '" + columns[0] + "'.");
            continue;
        }

        Course course;
        course.courseNumber = courseId;
        course.courseName = columns[1];

        std::set<std::string> seenPrereqs;
        for (std::size_t i = 2; i < columns.size(); ++i) {
            if (columns[i].empty()) {
                continue;
            }
            std::string prereqId = toUpper(columns[i]);
            if (!isCourseIdValid(prereqId)) {
                warnings.emplace_back("Skipping invalid prerequisite '" + columns[i] +
                                      "' for course " + course.courseNumber + ".");
                continue;
            }
            if (!seenPrereqs.insert(prereqId).second) {
                warnings.emplace_back("Duplicate prerequisite '" + prereqId +
                                      "' ignored for course " + course.courseNumber + ".");
                continue;
            }
            course.prerequisites.push_back(std::move(prereqId));
        }

        if (auto existing = loadedCourseDirectory.find(course.courseNumber); existing != loadedCourseDirectory.end()) {
            warnings.emplace_back("Replacing existing course entry for " + course.courseNumber + ".");
        }

        loadedCourseDirectory[course.courseNumber] = std::move(course);
    }

    if (loadedCourseDirectory.empty()) {
        result.warnings.insert(result.warnings.end(), warnings.begin(), warnings.end());
        return result;
    }

    // Capture prerequisites that refer to courses missing from the loaded catalog.
    std::set<std::string> missingSet;
    for (const auto& [courseId, course] : loadedCourseDirectory) {
        for (const auto& prereq : course.prerequisites) {
            if (loadedCourseDirectory.find(prereq) == loadedCourseDirectory.end()) {
                missingSet.insert(prereq + " (referenced by " + courseId + ")");
            }
        }
    }

    // Build the sorted course list once so lookups and listings stay fast.
    std::vector<std::string> sortedIds;
    sortedIds.reserve(loadedCourseDirectory.size());
    for (const auto& entry : loadedCourseDirectory) {
        sortedIds.push_back(entry.first);
    }
    std::sort(sortedIds.begin(), sortedIds.end());

    result.ok = true;
    result.courses = loadedCourseDirectory.size();
    result.missingPrerequisites.assign(missingSet.begin(), missingSet.end());
    result.warnings.insert(result.warnings.end(), warnings.begin(), warnings.end());

    courseDirectory = std::move(loadedCourseDirectory);
    sortedCourseIds = std::move(sortedIds);

    return result;
}

const Course* Catalog::get(const std::string& id) const {
    const auto it = courseDirectory.find(id);
    if (it == courseDirectory.end()) {
        return nullptr;
    }
    return &it->second;
}

std::vector<std::string> Catalog::ids() const {
    return sortedCourseIds;
}
