#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

// Represents a single course entry including the ID, title, and prerequisite IDs.
struct Course {
    std::string courseNumber;
    std::string courseName;
    std::vector<std::string> prerequisites;
};

// Collects the outcome from a catalog load attempt so callers can report results.
struct LoadResult {
    bool ok = false;
    std::size_t courses = 0;
    std::vector<std::string> warnings;
    std::vector<std::string> missingPrerequisites;
    std::string path;
};

class Catalog {
public:
    /**
     * Reads the CSV file, validates the data, and populates the in-memory catalog.
     * No output occurs here; the caller should surface the messages from the result.
     */
    LoadResult load(const std::string& fileName);

    /**
     * Finds a course by ID (case-sensitive to match the normalized entries).
     * Returns nullptr when the course is not in the catalog.
     */
    const Course* get(const std::string& id) const;

    /**
     * Provides a sorted list of every course ID currently loaded.
     * A copy is returned so callers can reuse or filter independently.
     */
    std::vector<std::string> ids() const;

private:
    std::unordered_map<std::string, Course> courseDirectory;
    std::vector<std::string> sortedCourseIds;
};
