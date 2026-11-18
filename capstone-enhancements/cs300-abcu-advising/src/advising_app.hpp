#ifndef ABCU_ADVISING_APP_HPP
#define ABCU_ADVISING_APP_HPP

#include <iosfwd>

namespace abcu {

// Menu choices. The numeric values are what the user types.
enum class MenuOption {
    LoadData = 1,
    PrintCourseList = 2,
    PrintCourse = 3,
    ValidatePrerequisites = 4,
    PrintStudyOrder = 5,
    Exit = 9
};

// Runs the interactive menu loop against the given streams, so tests can drive
// it with scripted input. Returns when the user chooses Exit or the input ends.
void runAdvisingApp(std::istream& in, std::ostream& out);

}  // namespace abcu

#endif
