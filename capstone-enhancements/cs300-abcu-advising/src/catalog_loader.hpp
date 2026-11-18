#ifndef ABCU_CATALOG_LOADER_HPP
#define ABCU_CATALOG_LOADER_HPP

#include <cstddef>
#include <string>
#include <vector>

#include "course_store.hpp"

namespace abcu {

struct LoadResult {
    bool opened = false;                 // false when the file could not be read
    std::string error;                   // set when opened is false
    std::size_t rowsRead = 0;            // non-blank lines seen
    std::size_t coursesLoaded = 0;       // distinct courses now in the store
    std::vector<std::string> warnings;   // skipped or replaced rows
};

// Replaces the contents of `store` with the catalog in `csvText`
// (courseNumber,courseName,prerequisite,prerequisite,...). Parsing is done by
// the csv::Parser from the earlier CS-300 assignments (third_party/csvparser).
LoadResult loadCatalogFromText(const std::string& csvText, CourseStore& store);

// Same, reading the text from a file first.
LoadResult loadCatalogFromFile(const std::string& path, CourseStore& store);

}  // namespace abcu

#endif
