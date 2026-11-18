#ifndef ABCU_COURSE_HPP
#define ABCU_COURSE_HPP

#include <string>
#include <vector>

namespace abcu {

// One catalog entry. Prerequisites are stored as course numbers exactly as
// they appear in the catalog file; the PrerequisiteGraph resolves them.
struct Course {
    std::string courseNumber;
    std::string courseName;
    std::vector<std::string> prerequisites;
};

// Course numbers are matched case-insensitively, so every store and the
// graph share this one normalization instead of each keeping its own copy.
std::string normalizeKey(const std::string& courseNumber);

}  // namespace abcu

#endif
