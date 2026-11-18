// Wall-clock comparison of the two CourseStore implementations.
//
//   abcu_benchmark                     real ABCU catalog only (8 courses)
//   abcu_benchmark --keys keys.tsv     also a scaling run on real keys from a
//                                      tab-separated file: key<TAB>name per line
//
// Timing is std::chrono::steady_clock around the exact operation, repeated, and
// the fastest repeat is reported (the least disturbed by the rest of the machine).
// Build in Release; a Debug build measures the debug runtime, not the structures.

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "catalog_loader.hpp"
#include "course_store.hpp"

#ifndef ABCU_DATA_FILE
#error "ABCU_DATA_FILE must point at data/ABCU_Advising_Program_Input.csv"
#endif

namespace {

using Clock = std::chrono::steady_clock;

double elapsedNs(Clock::time_point start) {
    return std::chrono::duration<double, std::nano>(Clock::now() - start).count();
}

// Keeps the optimizer from deleting lookups whose results are otherwise unused.
volatile std::size_t sink = 0;

struct Row {
    const char* label;
    double loadNsPerCourse;
    double hitNsPerLookup;
    double missNsPerLookup;
    double listNsPerCourse;
};

template <typename Make>
Row measure(const char* label, Make make, const std::vector<abcu::Course>& courses, int repeats) {
    std::vector<std::string> hitKeys;
    std::vector<std::string> missKeys;
    for (const auto& c : courses) {
        hitKeys.push_back(c.courseNumber);
        missKeys.push_back(c.courseNumber + "-ABSENT");
    }

    double bestLoad = 1e300, bestHit = 1e300, bestMiss = 1e300, bestList = 1e300;
    for (int r = 0; r < repeats; ++r) {
        auto store = make();

        auto t = Clock::now();
        for (const auto& c : courses) {
            store->insert(c);
        }
        bestLoad = std::min(bestLoad, elapsedNs(t));

        t = Clock::now();
        std::size_t found = 0;
        for (const auto& key : hitKeys) {
            found += store->find(key) != nullptr;
        }
        bestHit = std::min(bestHit, elapsedNs(t));
        sink = found;

        t = Clock::now();
        found = 0;
        for (const auto& key : missKeys) {
            found += store->find(key) != nullptr;
        }
        bestMiss = std::min(bestMiss, elapsedNs(t));
        sink = found;

        t = Clock::now();
        sink = store->allSorted().size();
        bestList = std::min(bestList, elapsedNs(t));
    }
    const double n = static_cast<double>(courses.size());
    return {label, bestLoad / n, bestHit / n, bestMiss / n, bestList / n};
}

void printHeader() {
    std::printf("  %-26s %14s %14s %14s %14s\n", "structure", "load ns/course", "hit ns/find",
                "miss ns/find", "list ns/course");
}

void printRow(const Row& row) {
    std::printf("  %-26s %14.1f %14.1f %14.1f %14.1f\n", row.label, row.loadNsPerCourse,
                row.hitNsPerLookup, row.missNsPerLookup, row.listNsPerCourse);
}

void compare(const std::vector<abcu::Course>& courses, int repeats) {
    printHeader();
    printRow(measure("ordered map (std::map)",
                     [] { return std::make_unique<abcu::OrderedCourseStore>(); }, courses, repeats));
    printRow(measure("hash table (chaining)",
                     [] { return std::make_unique<abcu::HashCourseStore>(); }, courses, repeats));
}

// The real ABCU catalog: load, hit lookups and miss lookups on the actual rows.
void benchmarkRealCatalog() {
    abcu::OrderedCourseStore source;
    const abcu::LoadResult loaded = abcu::loadCatalogFromFile(ABCU_DATA_FILE, source);
    std::vector<abcu::Course> courses;
    for (const abcu::Course* c : source.allSorted()) {
        courses.push_back(*c);
    }
    std::printf("Real ABCU catalog: %zu courses (%s)\n", loaded.coursesLoaded, ABCU_DATA_FILE);

    // One pass over 8 courses is below clock resolution, so each measurement
    // repeats the whole 8-course pass 20000 times and divides it back out.
    constexpr int kPasses = 20000;

    auto timePass = [&](const char* label, auto make) {
        double bestLoad = 1e300, bestHit = 1e300, bestMiss = 1e300;
        for (int repeat = 0; repeat < 5; ++repeat) {
            auto t = Clock::now();
            for (int pass = 0; pass < kPasses; ++pass) {
                auto store = make();
                for (const auto& c : courses) {
                    store->insert(c);
                }
                sink = store->size();
            }
            bestLoad = std::min(bestLoad, elapsedNs(t));

            auto store = make();
            for (const auto& c : courses) {
                store->insert(c);
            }
            t = Clock::now();
            std::size_t found = 0;
            for (int pass = 0; pass < kPasses; ++pass) {
                for (const auto& c : courses) {
                    found += store->find(c.courseNumber) != nullptr;
                }
            }
            bestHit = std::min(bestHit, elapsedNs(t));
            sink = found;

            t = Clock::now();
            found = 0;
            for (int pass = 0; pass < kPasses; ++pass) {
                for (const auto& c : courses) {
                    found += store->find(c.courseNumber + "-ABSENT") != nullptr;
                }
            }
            bestMiss = std::min(bestMiss, elapsedNs(t));
            sink = found;
        }
        const double perCourse = static_cast<double>(courses.size()) * kPasses;
        std::printf("  %-26s %14.1f %14.1f %14.1f\n", label, bestLoad / perCourse,
                    bestHit / perCourse, bestMiss / perCourse);
    };

    std::printf("  %-26s %14s %14s %14s\n", "structure", "load ns/course", "hit ns/find",
                "miss ns/find");
    timePass("ordered map (std::map)", [] { return std::make_unique<abcu::OrderedCourseStore>(); });
    timePass("hash table (chaining)", [] { return std::make_unique<abcu::HashCourseStore>(); });
    std::printf("\n");
}

std::vector<abcu::Course> readKeys(const std::string& path) {
    std::ifstream file(path);
    std::vector<abcu::Course> courses;
    std::string line;
    while (std::getline(file, line)) {
        const std::size_t tab = line.find('\t');
        if (tab == std::string::npos) {
            continue;
        }
        courses.push_back({line.substr(0, tab), line.substr(tab + 1), {}});
    }
    return courses;
}

void benchmarkScaling(const std::string& keysPath) {
    const std::vector<abcu::Course> all = readKeys(keysPath);
    if (all.empty()) {
        std::printf("No rows read from %s\n", keysPath.c_str());
        return;
    }
    std::printf("Scaling run on %zu real keys from %s\n", all.size(), keysPath.c_str());
    std::printf("(each cell is the fastest of 5 repeats; times are per course / per lookup)\n\n");

    std::vector<std::size_t> sizes = {1000, 5000, 10000, 25000, 50000};
    sizes.push_back(all.size());
    for (std::size_t n : sizes) {
        if (n > all.size()) {
            continue;
        }
        std::vector<abcu::Course> subset(all.begin(), all.begin() + static_cast<long>(n));
        std::printf("n = %zu\n", n);
        compare(subset, 5);

        abcu::HashCourseStore hashed;
        for (const auto& c : subset) {
            hashed.insert(c);
        }
        std::printf("  hash table shape: %zu buckets, load factor %.2f, longest chain %zu\n\n",
                    hashed.bucketCount(), hashed.loadFactor(), hashed.longestChain());
    }
}

}  // namespace

int main(int argc, char** argv) {
    std::string keysPath;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--keys") == 0 && i + 1 < argc) {
            keysPath = argv[++i];
        }
    }
    benchmarkRealCatalog();
    if (!keysPath.empty()) {
        benchmarkScaling(keysPath);
    }
    return 0;
}
