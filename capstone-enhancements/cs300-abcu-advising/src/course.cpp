#include "course.hpp"

#include <algorithm>
#include <cctype>

namespace abcu {

std::string normalizeKey(const std::string& courseNumber) {
    std::string key = courseNumber;
    std::transform(key.begin(), key.end(), key.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return key;
}

}  // namespace abcu
