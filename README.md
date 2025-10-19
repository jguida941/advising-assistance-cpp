# Course Advisor Dashboard

This project started as a school assignment where the goal was to build a simple C++ console application that could help academic advisors explore course prerequisites. The original rubric expected a command-line program that:

- Prompted for a CSV file name and loaded course data into a chosen data structure.
- Presented a text menu with options to:
  - Load the catalog file.
  - Print an alphanumeric list of Computer Science courses (including math support classes).
  - Look up a specific course and display its title and prerequisite list.
  - Exit the program.
- Printed the full course list in alphanumeric order.
- Printed detailed information (title and prerequisites) for any course the user requested.
- Applied standard best practices: error handling for invalid input, readable naming, and inline comments to explain the logic

## Course Advisor CLI
<img width="759" height="331" alt="Screenshot 2025-10-19 at 4 19 57 AM" src="https://github.com/user-attachments/assets/6a151e59-5f75-45f0-8cc5-3148ab763268" />

<img width="550" height="537" alt="Screenshot 2025-10-19 at 4 20 33 AM" src="https://github.com/user-attachments/assets/cabcce1d-2e9a-43ab-8422-34f827a68871" />

<img width="509" height="336" alt="Screenshot 2025-10-19 at 4 21 24 AM" src="https://github.com/user-attachments/assets/36f152cc-b900-480e-8571-81e9a346cb0d" />

## Swapping Course Advisor Menu Style

<img width="859" height="355" alt="Screenshot 2025-10-19 at 4 24 19 AM" src="https://github.com/user-attachments/assets/e13cb08e-0acb-47f5-b619-3f3f58b4220f" />

## Switching to Light Mode:

<img width="466" height="189" alt="Screenshot 2025-10-19 at 4 27 48 AM" src="https://github.com/user-attachments/assets/2c26a731-88f7-4717-a842-24869ed766d0" />

## Course Advisor QT Dashboard

<img width="955" height="628" alt="Screenshot 2025-10-19 at 4 19 30 AM" src="https://github.com/user-attachments/assets/f3d9cd35-19e8-4a13-9a9c-ea0e7ef09cfd" />




## Enhancements Beyond the Rubric

After completing the required CLI features, the project was extended substantially:

- **Shared Catalog Core:** Refactored all parsing, validation, and sorting logic into a reusable library so both binaries stay in sync.
- **Modernized CLI:** Kept the menu-driven console experience intact while adding a launch option for the new GUI dashboard and improving error feedback.
- **Qt Dashboard:** Built a Qt Widgets front end that mirrors the console features and adds richer interactivity:
  - File→Open and Reload actions for quick catalog switching.
  - Course list view with search-as-you-type support and prerequisite navigation.
  - Warning panel to surface loader issues without blocking the UI.
- **CMake Targets:** Split the build into three targets for clarity—`catalog_core`, `advisor_cli`, and `advisor_gui`.
- **Terminal Themes:** Added environment-driven customization for the CLI menu (see below).

These additions maintain the spirit of the assignment while making the tool more approachable for advisors who prefer a graphical interface.

## Design Choices

- **Hashtable-backed catalog:** The core catalog stores courses in a `std::unordered_map` (`src/catalog/catalog.cpp`) so prerequisite lookups stay `O(1)` regardless of catalog size. IDs are normalized to uppercase on load, which keeps the hash keys consistent between the CLI and GUI.
- **Cached sorted view:** Alongside the hash table, the loader materializes a `std::vector<std::string>` of course IDs once and reuses it for list rendering and search suggestions. This avoids resorting on every request and keeps the GUI model lightweight.
- **Load result telemetry:** The `LoadResult` struct bundles success state, warning messages, and missing prerequisites so every front end can surface the same diagnostics without re-reading the file.
- **GUI state caching:** The Qt layer retains the last-opened path and most recent `LoadResult`, enabling `Reload` and warning banners without additional disk work.
- **Build caching:** The CMake toolchain is configured for `ccache`, significantly cutting compile times as the project grows (mirrored in the GitHub Actions plan).

## Project Layout

```
final_project/
├── CMakeLists.txt
├── README.md
├── data/
│   └── CS 300 ABCU_Advising_Program_Input.csv
├── include/
│   ├── catalog/
│   │   └── catalog.hpp
│   └── gui/
│       ├── mainwindow.hpp
│       └── models.hpp
└── src/
    ├── catalog/
    │   └── catalog.cpp
    ├── cli/
    │   └── main_cli.cpp
    └── gui/
        ├── main_gui.cpp
        ├── mainwindow.cpp
        └── models.cpp
```

The sample course data now lives under `data/`, and the CLI defaults to `data/CS 300 ABCU_Advising_Program_Input.csv` when no path is supplied.

## Building and Running

This project uses CMake and requires a C++20 compiler plus Qt 6 Widgets for the GUI target.

```bash
# from the project folder, e.g. cd /Users/you/projects/final_project
# configure (creates a ./build directory containing the CMake files)
cmake -S . -B build

# build everything (drops advisor_cli and advisor_gui into ./build/)
cmake --build build
```

### Run the Console Advisor

```bash
./build/advisor_cli         # run from the repo root so ./build is found
# assignment alias still available as ./build/final_project
```

From the menu you can load a catalog, list courses, inspect prerequisites, or launch the Qt dashboard (option 4). If you already loaded a CSV, option 4 forwards that file to the GUI.

### Run the Qt Dashboard Directly

```bash
./build/advisor_gui            # macOS/Linux (run from the repo root)
./build/advisor_gui.exe        # Windows
```

You can optionally pass a CSV path to pre-load the catalog:

```bash
./build/advisor_gui "CS 300 ABCU_Advising_Program_Input.csv"
```

## CLI Appearance Tweaks

Several environment variables control how the console menu looks:

```bash
# Disable colours entirely (assumes the binary is in ./build/ after cmake --build)
NO_COLOR=1 ./build/advisor_cli

# Force a different colour palette (dark is the default)
COURSE_ADVISOR_THEME=light ./build/advisor_cli
COURSE_ADVISOR_THEME=plain ./build/advisor_cli   # keep colours off without touching NO_COLOR

# Swap the menu frame style
COURSE_ADVISOR_FRAME=unicode ./build/advisor_cli
COURSE_ADVISOR_FRAME=none ./build/advisor_cli

# Windows (PowerShell) examples — cd into the repo so build\ exists
cmd /c "cd /d C:\\path\\to\\final_project && set NO_COLOR=1 && build\\advisor_cli.exe"
powershell -Command "Set-Location C:\\path\\to\\final_project; $env:COURSE_ADVISOR_THEME='light'; .\\build\\advisor_cli.exe"
```

> If you are using an IDE-generated build directory (for example, `cmake-build-debug` in CLion), substitute that folder instead of `build/` in the commands above.

## Testing

The CLI remains the quickest way to verify behavior while iterating on the CSV parser:

1. Run `advisor_cli`.
2. Option `1` → load a catalog file.
3. Option `2` → confirm the sorted course list.
4. Option `3` → look up a few sample course IDs (valid and invalid) to check prerequisite handling.

Additional GUI validation:

- File→Open should refresh the catalog without restarting.
- Typing in the search field highlights and scrolls to matching courses.
- The warning panel appears only when the loader reports issues.
