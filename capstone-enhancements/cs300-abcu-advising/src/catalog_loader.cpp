#include "catalog_loader.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>

#include "CSVparser.hpp"

namespace abcu {

namespace {

constexpr unsigned kCourseNumberColumn = 0;
constexpr unsigned kCourseNameColumn = 1;
constexpr unsigned kFirstPrerequisiteColumn = 2;
constexpr unsigned kMinimumColumns = 2;

std::string trim(const std::string& text) {
    const char* whitespace = " \t\r\n";
    const std::size_t first = text.find_first_not_of(whitespace);
    if (first == std::string::npos) {
        return "";
    }
    const std::size_t last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
}

std::size_t columnsIn(const std::string& line) {
    return static_cast<std::size_t>(std::count(line.begin(), line.end(), ',')) + 1;
}

}  // namespace

LoadResult loadCatalogFromText(const std::string& csvText, CourseStore& store) {
    LoadResult result;
    result.opened = true;
    store.clear();

    // The catalog file has no header row, but csv::Parser always treats line 1
    // as the header and requires every row to have the same number of columns.
    // So: collect the non-blank lines, work out the widest row, prepend a
    // synthetic header of that width, and pad shorter rows with empty columns.
    // Rows with fewer than two columns (no course name) are reported and dropped.
    std::vector<std::string> lines;
    std::size_t width = kMinimumColumns;
    std::istringstream input(csvText);
    std::string line;
    while (std::getline(input, line)) {
        line = trim(line);
        if (line.empty()) {
            continue;
        }
        ++result.rowsRead;
        if (columnsIn(line) < kMinimumColumns) {
            result.warnings.push_back("Skipped row " + std::to_string(result.rowsRead) +
                                      " (needs a course number and a course name): " + line);
            continue;
        }
        width = std::max(width, columnsIn(line));
        lines.push_back(line);
    }

    std::string header = "courseNumber,courseName";
    for (std::size_t i = kFirstPrerequisiteColumn; i < width; ++i) {
        header += ",prerequisite" + std::to_string(i - kFirstPrerequisiteColumn + 1);
    }
    std::string padded = header + "\n";
    for (const std::string& row : lines) {
        padded += row + std::string(width - columnsIn(row), ',') + "\n";
    }

    try {
        csv::Parser parser(padded, csv::ePURE);
        for (unsigned i = 0; i < parser.rowCount(); ++i) {
            const csv::Row& row = parser.getRow(i);
            Course course;
            course.courseNumber = trim(row[kCourseNumberColumn]);
            course.courseName = trim(row[kCourseNameColumn]);
            if (course.courseNumber.empty() || course.courseName.empty()) {
                result.warnings.push_back("Skipped a row with an empty course number or name: " +
                                          lines[i]);
                continue;
            }
            for (unsigned col = kFirstPrerequisiteColumn; col < row.size(); ++col) {
                const std::string prerequisite = trim(row[col]);
                if (!prerequisite.empty()) {
                    course.prerequisites.push_back(prerequisite);
                }
            }
            if (store.find(course.courseNumber) != nullptr) {
                result.warnings.push_back("Course " + course.courseNumber +
                                          " appears more than once; the later row replaces the earlier one.");
            }
            store.insert(course);
        }
    } catch (const csv::Error& e) {
        result.warnings.push_back(std::string("Catalog could not be parsed: ") + e.what());
    }

    result.coursesLoaded = store.size();
    return result;
}

LoadResult loadCatalogFromFile(const std::string& path, CourseStore& store) {
    std::ifstream file(path);
    if (!file.is_open()) {
        LoadResult result;
        result.opened = false;
        result.error = "Could not open file " + path;
        return result;
    }
    std::stringstream contents;
    contents << file.rdbuf();
    return loadCatalogFromText(contents.str(), store);
}

}  // namespace abcu
