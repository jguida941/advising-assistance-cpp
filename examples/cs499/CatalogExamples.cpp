#include "catalog/catalog.hpp"

#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Provide one catalog file.\n";
        return 2;
    }
    Catalog catalog;
    const LoadResult result = catalog.load(argv[1]);
    std::cout << std::boolalpha;
    std::cout << "Actual load accepted: " << result.ok << '\n';
    std::cout << "Actual course count: " << result.courses << '\n';
    std::cout << "Actual warning count: " << result.warnings.size() << '\n';
    for (const auto& warning : result.warnings) {
        std::cout << "Warning: " << warning << '\n';
    }
    std::cout << "Actual missing prerequisite count: " << result.missingPrerequisites.size() << '\n';
    for (const auto& id : catalog.ids()) {
        const Course* course = catalog.get(id);
        if (course == nullptr) {
            std::cerr << "A listed course could not be retrieved.\n";
            return 2;
        }
        std::cout << "Course " << course->courseNumber << ": title = [" << course->courseName << "]";
        std::cout << "; prerequisites = [";
        for (std::size_t i = 0; i < course->prerequisites.size(); ++i) {
            if (i != 0) std::cout << ", ";
            std::cout << course->prerequisites[i];
        }
        std::cout << "]\n";
    }
    std::cout << "The rows above use the existing sorted display order, not a computed prerequisite schedule.\n";
    return 0;
}
