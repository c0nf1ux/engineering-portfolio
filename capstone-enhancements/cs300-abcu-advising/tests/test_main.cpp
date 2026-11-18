// Unit tests for the ABCU advising enhancement. No framework: a few macros and
// a registry keep the project free of downloads. Run `abcu_tests`; it exits
// non-zero if any check fails.
//
// Fixtures are built from the real catalog (data/ABCU_Advising_Program_Input.csv)
// wherever a test needs catalog content; the graph tests that need a broken
// catalog edit a line of the real file instead of inventing a new one.

#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "advising_app.hpp"
#include "catalog_loader.hpp"
#include "course_store.hpp"
#include "prerequisite_graph.hpp"

#ifndef ABCU_DATA_FILE
#error "ABCU_DATA_FILE must point at data/ABCU_Advising_Program_Input.csv"
#endif

namespace {

struct TestCase {
    std::string name;
    std::function<void()> body;
};

std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

int failures = 0;

struct Registrar {
    Registrar(const std::string& name, std::function<void()> body) {
        registry().push_back({name, std::move(body)});
    }
};

#define TEST(name)                                         \
    static void name();                                    \
    static Registrar registrar_##name(#name, name);        \
    static void name()

#define CHECK(condition)                                                              \
    do {                                                                              \
        if (!(condition)) {                                                           \
            ++failures;                                                               \
            std::cerr << "    FAILED " << __FILE__ << ":" << __LINE__ << ": " #condition \
                      << std::endl;                                                   \
        }                                                                             \
    } while (0)

std::string realCatalogText() {
    std::ifstream file(ABCU_DATA_FILE);
    std::stringstream text;
    text << file.rdbuf();
    return text.str();
}

std::string replaceFirstLine(const std::string& text, const std::string& newFirstLine) {
    return newFirstLine + text.substr(text.find('\n'));
}

std::string removeLinesStartingWith(const std::string& text, const std::string& prefix) {
    std::istringstream in(text);
    std::string line, result;
    while (std::getline(in, line)) {
        if (line.rfind(prefix, 0) != 0) {
            result += line + "\n";
        }
    }
    return result;
}

std::vector<std::unique_ptr<abcu::CourseStore>> bothStores() {
    std::vector<std::unique_ptr<abcu::CourseStore>> stores;
    stores.push_back(std::make_unique<abcu::OrderedCourseStore>());
    stores.push_back(std::make_unique<abcu::HashCourseStore>());
    return stores;
}

std::size_t indexOf(const std::vector<std::string>& list, const std::string& value) {
    for (std::size_t i = 0; i < list.size(); ++i) {
        if (list[i] == value) {
            return i;
        }
    }
    return list.size();
}

std::string runApp(const std::string& input) {
    std::istringstream in(input);
    std::ostringstream out;
    abcu::runAdvisingApp(in, out);
    return out.str();
}

bool contains(const std::string& text, const std::string& part) {
    return text.find(part) != std::string::npos;
}

// ---- parsing ---------------------------------------------------------------

TEST(parsing_loads_every_row_of_the_real_catalog) {
    abcu::OrderedCourseStore store;
    const abcu::LoadResult result = abcu::loadCatalogFromFile(ABCU_DATA_FILE, store);
    CHECK(result.opened);
    CHECK(result.coursesLoaded == 8);
    CHECK(result.warnings.empty());
}

TEST(parsing_keeps_prerequisites_in_file_order) {
    abcu::OrderedCourseStore store;
    abcu::loadCatalogFromFile(ABCU_DATA_FILE, store);
    const abcu::Course* course = store.find("CSCI300");
    CHECK(course != nullptr);
    if (course != nullptr) {
        CHECK(course->courseName == "Introduction to Algorithms");
        CHECK(course->prerequisites.size() == 2);
        CHECK(course->prerequisites.size() == 2 && course->prerequisites[0] == "CSCI200");
        CHECK(course->prerequisites.size() == 2 && course->prerequisites[1] == "MATH201");
    }
}

TEST(parsing_course_without_prerequisites_has_an_empty_list) {
    abcu::OrderedCourseStore store;
    abcu::loadCatalogFromFile(ABCU_DATA_FILE, store);
    const abcu::Course* course = store.find("CSCI100");
    CHECK(course != nullptr && course->prerequisites.empty());
}

TEST(parsing_pads_rows_with_fewer_columns_than_the_widest_row) {
    abcu::OrderedCourseStore store;
    const abcu::LoadResult result = abcu::loadCatalogFromText(
        "CSCI100,Introduction to Computer Science\n"
        "CSCI101,Introduction to Programming in C++,CSCI100\n",
        store);
    CHECK(result.coursesLoaded == 2);
    CHECK(result.warnings.empty());
    CHECK(store.find("CSCI101") != nullptr && store.find("CSCI101")->prerequisites.size() == 1);
}

TEST(parsing_handles_windows_line_endings_and_blank_lines) {
    abcu::OrderedCourseStore store;
    const abcu::LoadResult result = abcu::loadCatalogFromText(
        "CSCI100,Introduction to Computer Science,,\r\n\r\nCSCI101,Introduction to Programming in C++,CSCI100,\r\n",
        store);
    CHECK(result.coursesLoaded == 2);
    CHECK(store.find("CSCI101") != nullptr && store.find("CSCI101")->prerequisites.size() == 1);
}

TEST(parsing_reports_a_row_with_no_course_name_and_keeps_going) {
    abcu::OrderedCourseStore store;
    const abcu::LoadResult result = abcu::loadCatalogFromText(
        "CSCI100\nCSCI101,Introduction to Programming in C++,CSCI100\n", store);
    CHECK(result.coursesLoaded == 1);
    CHECK(result.warnings.size() == 1);
}

TEST(parsing_reports_a_duplicate_course_number_and_counts_it_once) {
    abcu::OrderedCourseStore store;
    const abcu::LoadResult result = abcu::loadCatalogFromText(
        "CSCI100,Introduction to Computer Science\ncsci100,Intro to CS (revised)\n", store);
    CHECK(result.coursesLoaded == 1);
    CHECK(result.warnings.size() == 1);
    CHECK(store.find("CSCI100")->courseName == "Intro to CS (revised)");
}

TEST(parsing_missing_file_reports_an_error_instead_of_throwing) {
    abcu::OrderedCourseStore store;
    const abcu::LoadResult result = abcu::loadCatalogFromFile("no_such_catalog_file.csv", store);
    CHECK(!result.opened);
    CHECK(!result.error.empty());
}

TEST(parsing_replaces_the_previous_contents_of_the_store) {
    abcu::OrderedCourseStore store;
    abcu::loadCatalogFromFile(ABCU_DATA_FILE, store);
    abcu::loadCatalogFromText("CSCI100,Introduction to Computer Science\n", store);
    CHECK(store.size() == 1);
    CHECK(store.find("CSCI300") == nullptr);
}

// ---- lookup, both data structures -----------------------------------------

TEST(lookup_is_case_insensitive_in_both_stores) {
    for (auto& store : bothStores()) {
        abcu::loadCatalogFromFile(ABCU_DATA_FILE, *store);
        CHECK(store->find("csci200") != nullptr);
        CHECK(store->find("CsCi200") != nullptr);
        CHECK(store->find("CSCI200")->courseName == "Data Structures");
    }
}

TEST(lookup_of_an_unknown_course_returns_null_in_both_stores) {
    for (auto& store : bothStores()) {
        abcu::loadCatalogFromFile(ABCU_DATA_FILE, *store);
        CHECK(store->find("CSCI999") == nullptr);
        CHECK(store->find("") == nullptr);
    }
}

TEST(insert_with_an_existing_key_replaces_in_both_stores) {
    for (auto& store : bothStores()) {
        store->insert({"CSCI100", "Original", {}});
        store->insert({"csci100", "Replacement", {}});
        CHECK(store->size() == 1);
        CHECK(store->find("CSCI100")->courseName == "Replacement");
    }
}

TEST(listing_is_sorted_by_course_number_in_both_stores) {
    for (auto& store : bothStores()) {
        abcu::loadCatalogFromFile(ABCU_DATA_FILE, *store);
        const auto sorted = store->allSorted();
        CHECK(sorted.size() == 8);
        for (std::size_t i = 1; i < sorted.size(); ++i) {
            CHECK(abcu::normalizeKey(sorted[i - 1]->courseNumber) <
                  abcu::normalizeKey(sorted[i]->courseNumber));
        }
    }
}

TEST(both_stores_return_identical_listings_for_the_real_catalog) {
    abcu::OrderedCourseStore ordered;
    abcu::HashCourseStore hashed;
    abcu::loadCatalogFromFile(ABCU_DATA_FILE, ordered);
    abcu::loadCatalogFromFile(ABCU_DATA_FILE, hashed);
    const auto a = ordered.allSorted();
    const auto b = hashed.allSorted();
    CHECK(a.size() == b.size());
    for (std::size_t i = 0; i < a.size() && i < b.size(); ++i) {
        CHECK(a[i]->courseNumber == b[i]->courseNumber);
        CHECK(a[i]->prerequisites == b[i]->prerequisites);
    }
}

TEST(hash_table_grows_and_keeps_every_entry_findable) {
    // The key range is only an input to force several rehashes; the lookups
    // check that nothing is lost or duplicated when the table doubles.
    abcu::HashCourseStore store(4);
    const std::size_t startingBuckets = store.bucketCount();
    for (int i = 0; i < 5000; ++i) {
        store.insert({"CSCI" + std::to_string(i), "Course " + std::to_string(i), {}});
    }
    CHECK(store.size() == 5000);
    CHECK(store.bucketCount() > startingBuckets);
    CHECK(store.loadFactor() <= 0.75);
    for (int i = 0; i < 5000; ++i) {
        CHECK(store.find("csci" + std::to_string(i)) != nullptr);
    }
    CHECK(store.find("CSCI5000") == nullptr);
}

TEST(hash_table_clear_empties_the_table) {
    abcu::HashCourseStore store;
    abcu::loadCatalogFromFile(ABCU_DATA_FILE, store);
    store.clear();
    CHECK(store.size() == 0);
    CHECK(store.find("CSCI100") == nullptr);
    CHECK(store.allSorted().empty());
}

// ---- prerequisite graph ----------------------------------------------------

TEST(graph_real_catalog_is_valid) {
    abcu::OrderedCourseStore store;
    abcu::loadCatalogFromFile(ABCU_DATA_FILE, store);
    const abcu::ValidationReport report = abcu::PrerequisiteGraph(store).validate();
    CHECK(report.valid());
    CHECK(report.missingPrerequisites.empty());
    CHECK(report.cycle.empty());
}

TEST(graph_study_order_puts_every_prerequisite_first) {
    abcu::OrderedCourseStore store;
    abcu::loadCatalogFromFile(ABCU_DATA_FILE, store);
    const abcu::StudyOrder order = abcu::PrerequisiteGraph(store).studyOrder();
    CHECK(order.complete);
    CHECK(order.courses.size() == 8);
    for (const abcu::Course* course : store.allSorted()) {
        const std::size_t position = indexOf(order.courses, course->courseNumber);
        CHECK(position < order.courses.size());
        for (const std::string& prerequisite : course->prerequisites) {
            CHECK(indexOf(order.courses, prerequisite) < position);
        }
    }
}

TEST(graph_study_order_is_deterministic_and_matches_both_stores) {
    abcu::OrderedCourseStore ordered;
    abcu::HashCourseStore hashed;
    abcu::loadCatalogFromFile(ABCU_DATA_FILE, ordered);
    abcu::loadCatalogFromFile(ABCU_DATA_FILE, hashed);
    const std::vector<std::string> expected = {"CSCI100", "CSCI101", "CSCI200", "CSCI301",
                                               "MATH201", "CSCI300", "CSCI350", "CSCI400"};
    CHECK(abcu::PrerequisiteGraph(ordered).studyOrder().courses == expected);
    CHECK(abcu::PrerequisiteGraph(hashed).studyOrder().courses == expected);
}

TEST(graph_finds_a_circular_chain_made_by_editing_one_real_line) {
    // The real file has MATH201 with no prerequisites. Making it require CSCI400
    // closes the loop MATH201 -> CSCI400 -> CSCI350 -> CSCI300 -> MATH201.
    abcu::OrderedCourseStore store;
    abcu::loadCatalogFromText(
        replaceFirstLine(realCatalogText(), "MATH201,Discrete Mathematics,CSCI400,"), store);
    const abcu::PrerequisiteGraph graph(store);
    const abcu::ValidationReport report = graph.validate();
    CHECK(!report.valid());
    CHECK(report.missingPrerequisites.empty());
    CHECK(report.cycle.size() == 5);
    CHECK(!report.cycle.empty() && report.cycle.front() == report.cycle.back());
    CHECK(indexOf(report.cycle, "MATH201") < report.cycle.size());
    CHECK(indexOf(report.cycle, "CSCI400") < report.cycle.size());
    CHECK(indexOf(report.cycle, "CSCI350") < report.cycle.size());
    CHECK(indexOf(report.cycle, "CSCI300") < report.cycle.size());
}

TEST(graph_every_consecutive_pair_in_a_reported_cycle_is_a_real_requirement) {
    abcu::OrderedCourseStore store;
    abcu::loadCatalogFromText(
        replaceFirstLine(realCatalogText(), "MATH201,Discrete Mathematics,CSCI400,"), store);
    const abcu::ValidationReport report = abcu::PrerequisiteGraph(store).validate();
    for (std::size_t i = 0; i + 1 < report.cycle.size(); ++i) {
        const abcu::Course* course = store.find(report.cycle[i]);
        CHECK(course != nullptr);
        if (course != nullptr) {
            CHECK(indexOf(course->prerequisites, report.cycle[i + 1]) < course->prerequisites.size());
        }
    }
}

TEST(graph_study_order_is_incomplete_when_there_is_a_cycle) {
    abcu::OrderedCourseStore store;
    abcu::loadCatalogFromText(
        replaceFirstLine(realCatalogText(), "MATH201,Discrete Mathematics,CSCI400,"), store);
    const abcu::StudyOrder order = abcu::PrerequisiteGraph(store).studyOrder();
    CHECK(!order.complete);
    // The four courses in the loop can never be placed; the four courses that
    // do not depend on the loop still can.
    CHECK(order.courses.size() == 4);
    CHECK(indexOf(order.courses, "CSCI100") < order.courses.size());
    CHECK(indexOf(order.courses, "CSCI101") < order.courses.size());
    CHECK(indexOf(order.courses, "CSCI200") < order.courses.size());
    CHECK(indexOf(order.courses, "CSCI301") < order.courses.size());
}

TEST(graph_a_course_that_requires_itself_is_a_cycle) {
    abcu::OrderedCourseStore store;
    abcu::loadCatalogFromText(
        replaceFirstLine(realCatalogText(), "MATH201,Discrete Mathematics,MATH201,"), store);
    const abcu::ValidationReport report = abcu::PrerequisiteGraph(store).validate();
    CHECK(report.cycle.size() == 2);
    CHECK(report.cycle.size() == 2 && report.cycle[0] == "MATH201" && report.cycle[1] == "MATH201");
}

TEST(graph_reports_a_prerequisite_that_is_not_in_the_catalog) {
    // Dropping CSCI200 from the real file leaves CSCI300 requiring a course that
    // is no longer in the catalog.
    abcu::OrderedCourseStore store;
    abcu::loadCatalogFromText(removeLinesStartingWith(realCatalogText(), "CSCI200"), store);
    const abcu::ValidationReport report = abcu::PrerequisiteGraph(store).validate();
    CHECK(!report.valid());
    CHECK(report.cycle.empty());
    CHECK(report.missingPrerequisites.size() == 1);
    CHECK(report.missingPrerequisites.size() == 1 &&
          report.missingPrerequisites[0].first == "CSCI300" &&
          report.missingPrerequisites[0].second == "CSCI200");
}

TEST(graph_missing_prerequisite_does_not_stop_the_study_order) {
    abcu::OrderedCourseStore store;
    abcu::loadCatalogFromText(removeLinesStartingWith(realCatalogText(), "CSCI200"), store);
    const abcu::StudyOrder order = abcu::PrerequisiteGraph(store).studyOrder();
    CHECK(order.complete);
    CHECK(order.courses.size() == 7);
}

TEST(graph_prerequisite_matching_ignores_case) {
    abcu::OrderedCourseStore store;
    abcu::loadCatalogFromText(
        "CSCI100,Introduction to Computer Science\ncsci101,Introduction to Programming in C++,Csci100\n",
        store);
    CHECK(abcu::PrerequisiteGraph(store).validate().valid());
}

TEST(graph_empty_catalog_is_valid_and_has_an_empty_order) {
    abcu::OrderedCourseStore store;
    const abcu::PrerequisiteGraph graph(store);
    CHECK(graph.validate().valid());
    CHECK(graph.studyOrder().courses.empty());
    CHECK(graph.studyOrder().complete);
}

// ---- menu and input handling ------------------------------------------------

TEST(menu_non_numeric_input_is_rejected_and_the_loop_still_exits) {
    // The original program read the choice with `cin >> int`. A letter put cin
    // into a failed state that was never cleared, so it printed the invalid-option
    // message forever. Here the bad line is consumed and "9" is reached.
    const std::string output = runApp("abc\n9\n");
    CHECK(contains(output, "\"abc\" is not a valid option."));
    CHECK(contains(output, "Thank you for using the course planner!"));
}

TEST(menu_input_that_ends_without_exit_does_not_loop_forever) {
    const std::string output = runApp("abc\n");
    CHECK(contains(output, "is not a valid option."));
}

TEST(menu_rejects_numbers_that_are_not_menu_options) {
    const std::string output = runApp("7\n-1\n12abc\n9\n");
    CHECK(contains(output, "7 is not a valid option."));
    CHECK(contains(output, "\"-1\" is not a valid option."));
    CHECK(contains(output, "\"12abc\" is not a valid option."));
}

TEST(menu_requires_data_before_listing) {
    const std::string output = runApp("2\n3\n4\n5\n9\n");
    std::size_t count = 0;
    for (std::size_t at = output.find("No data loaded"); at != std::string::npos;
         at = output.find("No data loaded", at + 1)) {
        ++count;
    }
    CHECK(count == 4);
}

TEST(menu_missing_file_is_reported_and_the_app_keeps_running) {
    const std::string output = runApp("1\nno_such_catalog_file.csv\n9\n");
    CHECK(contains(output, "Error: Could not open file no_such_catalog_file.csv"));
    CHECK(contains(output, "Thank you for using the course planner!"));
}

TEST(menu_full_session_on_the_real_catalog) {
    const std::string output =
        runApp(std::string("1\n") + ABCU_DATA_FILE + "\n2\n3\ncsci300\n4\n5\n9\n");
    CHECK(contains(output, "Data loaded successfully. 8 courses loaded."));
    CHECK(contains(output, "CSCI100, Introduction to Computer Science"));
    CHECK(contains(output, "Prerequisites: CSCI200, MATH201"));
    CHECK(contains(output, "All prerequisites are valid"));
    CHECK(contains(output, "8. CSCI400"));
}

TEST(menu_reports_a_circular_chain_after_loading_one) {
    // Write the edited real catalog beside the test binary and load it by name.
    const std::string path = "abcu_cycle_fixture.csv";
    {
        std::ofstream file(path);
        file << replaceFirstLine(realCatalogText(), "MATH201,Discrete Mathematics,CSCI400,");
    }
    const std::string output = runApp("1\n" + path + "\n4\n5\n9\n");
    std::remove(path.c_str());
    CHECK(contains(output, "the prerequisite data has problems"));
    CHECK(contains(output, "Circular prerequisite chain:"));
    CHECK(contains(output, "Not every course could be placed"));
}

}  // namespace

int main() {
    for (const TestCase& test : registry()) {
        const int before = failures;
        test.body();
        std::cout << (failures == before ? "[ OK ] " : "[FAIL] ") << test.name << std::endl;
    }
    std::cout << registry().size() << " tests, " << failures << " failed checks" << std::endl;
    return failures == 0 ? 0 : 1;
}
