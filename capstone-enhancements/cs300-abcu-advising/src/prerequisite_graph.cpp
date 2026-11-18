#include "prerequisite_graph.hpp"

#include <algorithm>
#include <functional>
#include <map>
#include <queue>

namespace abcu {

namespace {
enum class Visit { Unseen, InProgress, Done };
}

PrerequisiteGraph::PrerequisiteGraph(const CourseStore& store) {
    const std::vector<const Course*> courses = store.allSorted();

    std::map<std::string, std::size_t> indexByKey;
    for (std::size_t i = 0; i < courses.size(); ++i) {
        numbers_.push_back(courses[i]->courseNumber);
        indexByKey[normalizeKey(courses[i]->courseNumber)] = i;
    }

    requires_.resize(courses.size());
    for (std::size_t i = 0; i < courses.size(); ++i) {
        for (const std::string& prerequisite : courses[i]->prerequisites) {
            auto found = indexByKey.find(normalizeKey(prerequisite));
            if (found == indexByKey.end()) {
                missing_.emplace_back(courses[i]->courseNumber, prerequisite);
            } else {
                requires_[i].push_back(found->second);
            }
        }
        // A prerequisite listed twice is still one edge.
        std::sort(requires_[i].begin(), requires_[i].end());
        requires_[i].erase(std::unique(requires_[i].begin(), requires_[i].end()),
                           requires_[i].end());
    }
}

ValidationReport PrerequisiteGraph::validate() const {
    ValidationReport report;
    report.missingPrerequisites = missing_;

    const std::size_t n = numbers_.size();
    std::vector<Visit> state(n, Visit::Unseen);

    // Depth-first search with an explicit stack (no recursion, so a long
    // prerequisite chain cannot overflow the call stack). `path` is the current
    // chain of courses being explored; reaching a course that is already on
    // that chain means the chain loops back on itself.
    struct Frame {
        std::size_t node;
        std::size_t nextEdge;
    };
    for (std::size_t start = 0; start < n && report.cycle.empty(); ++start) {
        if (state[start] != Visit::Unseen) {
            continue;
        }
        std::vector<Frame> path;
        path.push_back({start, 0});
        state[start] = Visit::InProgress;

        while (!path.empty() && report.cycle.empty()) {
            Frame& top = path.back();
            if (top.nextEdge == requires_[top.node].size()) {
                state[top.node] = Visit::Done;
                path.pop_back();
                continue;
            }
            const std::size_t next = requires_[top.node][top.nextEdge++];
            if (state[next] == Visit::InProgress) {
                auto loopStart = std::find_if(path.begin(), path.end(),
                                              [next](const Frame& f) { return f.node == next; });
                for (auto it = loopStart; it != path.end(); ++it) {
                    report.cycle.push_back(numbers_[it->node]);
                }
                report.cycle.push_back(numbers_[next]);
            } else if (state[next] == Visit::Unseen) {
                state[next] = Visit::InProgress;
                path.push_back({next, 0});
            }
        }
    }
    return report;
}

StudyOrder PrerequisiteGraph::studyOrder() const {
    const std::size_t n = numbers_.size();

    std::vector<std::size_t> unmetPrerequisites(n, 0);
    std::vector<std::vector<std::size_t>> unlocks(n);
    for (std::size_t course = 0; course < n; ++course) {
        unmetPrerequisites[course] = requires_[course].size();
        for (std::size_t prerequisite : requires_[course]) {
            unlocks[prerequisite].push_back(course);
        }
    }

    // Min-heap on the node index, which is already sorted by course number.
    std::priority_queue<std::size_t, std::vector<std::size_t>, std::greater<std::size_t>> ready;
    for (std::size_t course = 0; course < n; ++course) {
        if (unmetPrerequisites[course] == 0) {
            ready.push(course);
        }
    }

    StudyOrder order;
    while (!ready.empty()) {
        const std::size_t course = ready.top();
        ready.pop();
        order.courses.push_back(numbers_[course]);
        for (std::size_t dependent : unlocks[course]) {
            if (--unmetPrerequisites[dependent] == 0) {
                ready.push(dependent);
            }
        }
    }
    order.complete = (order.courses.size() == n);
    return order;
}

}  // namespace abcu
