#include "advising_app.hpp"

#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <string>

#include "catalog_loader.hpp"
#include "course_store.hpp"
#include "prerequisite_graph.hpp"

namespace abcu {

namespace {

// Reads one whole line. Every prompt reads a line instead of using operator>>,
// so a bad entry is consumed and rejected instead of leaving the stream in a
// failed state that every later read trips over.
bool readLine(std::istream& in, std::string& line) {
    if (!std::getline(in, line)) {
        return false;
    }
    const std::size_t first = line.find_first_not_of(" \t\r");
    const std::size_t last = line.find_last_not_of(" \t\r");
    line = (first == std::string::npos) ? "" : line.substr(first, last - first + 1);
    return true;
}

// True only when the whole line is one integer.
bool parseInteger(const std::string& text, int& value) {
    if (text.empty()) {
        return false;
    }
    char* end = nullptr;
    errno = 0;
    const long parsed = std::strtol(text.c_str(), &end, 10);
    if (errno != 0 || *end != '\0' || parsed < 0 || parsed > 1000000) {
        return false;
    }
    value = static_cast<int>(parsed);
    return true;
}

void displayMenu(std::ostream& out) {
    out << static_cast<int>(MenuOption::LoadData) << ". Load Data Structure." << std::endl;
    out << static_cast<int>(MenuOption::PrintCourseList) << ". Print Course List." << std::endl;
    out << static_cast<int>(MenuOption::PrintCourse) << ". Print Course." << std::endl;
    out << static_cast<int>(MenuOption::ValidatePrerequisites) << ". Validate Prerequisites."
        << std::endl;
    out << static_cast<int>(MenuOption::PrintStudyOrder) << ". Print Study Order." << std::endl;
    out << static_cast<int>(MenuOption::Exit) << ". Exit" << std::endl;
}

void printCycle(std::ostream& out, const std::vector<std::string>& cycle) {
    out << "Circular prerequisite chain: ";
    for (std::size_t i = 0; i < cycle.size(); ++i) {
        out << cycle[i] << (i + 1 < cycle.size() ? " requires " : "");
    }
    out << std::endl;
}

void printReport(std::ostream& out, const ValidationReport& report) {
    for (const auto& missing : report.missingPrerequisites) {
        out << "Course " << missing.first << " requires " << missing.second
            << ", which is not in the catalog." << std::endl;
    }
    if (!report.cycle.empty()) {
        printCycle(out, report.cycle);
    }
}

class AdvisingApp {
public:
    AdvisingApp(std::istream& in, std::ostream& out) : in_(in), out_(out) {}

    void run() {
        out_ << "Welcome to the course planner." << std::endl;
        while (true) {
            displayMenu(out_);
            out_ << "What would you like to do? ";
            std::string line;
            if (!readLine(in_, line)) {
                out_ << std::endl;  // input ended; leave instead of looping forever
                return;
            }
            int choice = 0;
            if (!parseInteger(line, choice)) {
                out_ << "\"" << line << "\" is not a valid option." << std::endl;
            } else if (!handle(static_cast<MenuOption>(choice))) {
                out_ << "Thank you for using the course planner!" << std::endl;
                return;
            }
            out_ << std::endl;
        }
    }

private:
    // Returns false when the user chose Exit.
    bool handle(MenuOption option) {
        switch (option) {
        case MenuOption::LoadData:
            loadData();
            return true;
        case MenuOption::PrintCourseList:
            printCourseList();
            return true;
        case MenuOption::PrintCourse:
            printCourse();
            return true;
        case MenuOption::ValidatePrerequisites:
            validatePrerequisites();
            return true;
        case MenuOption::PrintStudyOrder:
            printStudyOrder();
            return true;
        case MenuOption::Exit:
            return false;
        }
        out_ << static_cast<int>(option) << " is not a valid option." << std::endl;
        return true;
    }

    bool requireData() {
        if (store_.size() == 0) {
            out_ << "No data loaded. Please load data first." << std::endl;
            return false;
        }
        return true;
    }

    void loadData() {
        out_ << "Enter the filename containing course data: ";
        std::string filename;
        if (!readLine(in_, filename) || filename.empty()) {
            out_ << "Error: no filename entered." << std::endl;
            return;
        }
        const LoadResult result = loadCatalogFromFile(filename, store_);
        if (!result.opened) {
            out_ << "Error: " << result.error << std::endl;
            return;
        }
        for (const std::string& warning : result.warnings) {
            out_ << "Warning: " << warning << std::endl;
        }
        out_ << "Data loaded successfully. " << result.coursesLoaded << " courses loaded."
             << std::endl;

        const ValidationReport report = PrerequisiteGraph(store_).validate();
        if (!report.valid()) {
            out_ << "Warning: the prerequisite data has problems. Choose option "
                 << static_cast<int>(MenuOption::ValidatePrerequisites) << " for details."
                 << std::endl;
        }
    }

    void printCourseList() {
        if (!requireData()) {
            return;
        }
        out_ << "Here is a sample schedule:" << std::endl;
        for (const Course* course : store_.allSorted()) {
            out_ << course->courseNumber << ", " << course->courseName << std::endl;
        }
    }

    void printCourse() {
        if (!requireData()) {
            return;
        }
        out_ << "What course do you want to know about? ";
        std::string courseNumber;
        if (!readLine(in_, courseNumber) || courseNumber.empty()) {
            out_ << "Error: no course number entered." << std::endl;
            return;
        }
        const Course* course = store_.find(courseNumber);
        if (course == nullptr) {
            out_ << "Course " << courseNumber << " not found." << std::endl;
            return;
        }
        out_ << course->courseNumber << ", " << course->courseName << std::endl;
        if (!course->prerequisites.empty()) {
            out_ << "Prerequisites: ";
            for (std::size_t i = 0; i < course->prerequisites.size(); ++i) {
                out_ << course->prerequisites[i];
                if (i + 1 < course->prerequisites.size()) {
                    out_ << ", ";
                }
            }
            out_ << std::endl;
        }
    }

    void validatePrerequisites() {
        if (!requireData()) {
            return;
        }
        const ValidationReport report = PrerequisiteGraph(store_).validate();
        if (report.valid()) {
            out_ << "All prerequisites are valid: every prerequisite is in the catalog and no "
                    "course depends on itself, directly or through other courses."
                 << std::endl;
        } else {
            printReport(out_, report);
        }
    }

    void printStudyOrder() {
        if (!requireData()) {
            return;
        }
        const PrerequisiteGraph graph(store_);
        const StudyOrder order = graph.studyOrder();
        out_ << "Courses in an order that satisfies every prerequisite:" << std::endl;
        for (std::size_t i = 0; i < order.courses.size(); ++i) {
            out_ << (i + 1) << ". " << order.courses[i] << std::endl;
        }
        if (!order.complete) {
            out_ << "Not every course could be placed because of a circular prerequisite chain. "
                    "Choose option "
                 << static_cast<int>(MenuOption::ValidatePrerequisites) << " for details."
                 << std::endl;
        }
    }

    std::istream& in_;
    std::ostream& out_;
    OrderedCourseStore store_;
};

}  // namespace

void runAdvisingApp(std::istream& in, std::ostream& out) {
    AdvisingApp(in, out).run();
}

}  // namespace abcu
