#ifndef ABCU_PREREQUISITE_GRAPH_HPP
#define ABCU_PREREQUISITE_GRAPH_HPP

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "course_store.hpp"

namespace abcu {

struct ValidationReport {
    // (course number, prerequisite that is not in the catalog)
    std::vector<std::pair<std::string, std::string>> missingPrerequisites;
    // One circular chain, e.g. {"A", "B", "C", "A"} meaning A requires B requires
    // C requires A. Empty when the catalog has no cycle.
    std::vector<std::string> cycle;

    bool valid() const { return missingPrerequisites.empty() && cycle.empty(); }
};

struct StudyOrder {
    // Prerequisites always appear before the courses that need them. When the
    // catalog has a cycle, only the courses outside every cycle (and not
    // downstream of one) can be ordered, so `complete` is false.
    std::vector<std::string> courses;
    bool complete = true;
};

// The catalog's prerequisites modelled as a directed graph: one node per
// course, one edge from a course to each prerequisite it requires.
class PrerequisiteGraph {
public:
    // Reads every course in the store. The graph holds its own copy of the
    // relationships, so the store can change afterwards without invalidating it.
    explicit PrerequisiteGraph(const CourseStore& store);

    // Finds prerequisites that do not exist in the catalog and, if there is one,
    // a circular chain. Iterative three-colour depth-first search, O(V + E).
    ValidationReport validate() const;

    // Kahn's algorithm, smallest course number first among ready courses, so
    // the output is deterministic. O((V + E) log V).
    StudyOrder studyOrder() const;

    std::size_t courseCount() const { return numbers_.size(); }

private:
    std::vector<std::string> numbers_;                         // display form, sorted by key
    std::vector<std::vector<std::size_t>> requires_;           // course -> prerequisite nodes
    std::vector<std::pair<std::string, std::string>> missing_;
};

}  // namespace abcu

#endif
