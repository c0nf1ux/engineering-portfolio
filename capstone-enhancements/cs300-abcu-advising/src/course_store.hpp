#ifndef ABCU_COURSE_STORE_HPP
#define ABCU_COURSE_STORE_HPP

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "course.hpp"

namespace abcu {

// Common interface so the ordered map and the hash table can be swapped in the
// application, the graph code, the unit tests, and the benchmark without any
// of them knowing which structure sits underneath.
class CourseStore {
public:
    virtual ~CourseStore() = default;

    virtual const char* name() const = 0;
    virtual void clear() = 0;
    // Inserts the course, or replaces the existing entry with the same key.
    virtual void insert(const Course& course) = 0;
    // Returns nullptr when the course number is not in the store.
    virtual const Course* find(const std::string& courseNumber) const = 0;
    virtual std::size_t size() const = 0;
    // Every course, ordered by normalized course number.
    virtual std::vector<const Course*> allSorted() const = 0;
};

// The original design: a balanced binary tree (std::map).
// insert/find O(log n); allSorted O(n) because the tree is already ordered.
class OrderedCourseStore : public CourseStore {
public:
    const char* name() const override { return "ordered map (std::map)"; }
    void clear() override { courses_.clear(); }
    void insert(const Course& course) override;
    const Course* find(const std::string& courseNumber) const override;
    std::size_t size() const override { return courses_.size(); }
    std::vector<const Course*> allSorted() const override;

private:
    std::map<std::string, Course> courses_;
};

// Separate chaining hash table, written from scratch (FNV-1a hash, power-of-two
// bucket count, doubles when the load factor passes 0.75).
// insert/find O(1) average, O(n) worst case; allSorted O(n log n) because a hash
// table keeps no order, so listing has to collect and sort.
class HashCourseStore : public CourseStore {
public:
    explicit HashCourseStore(std::size_t initialBuckets = 16);

    const char* name() const override { return "hash table (chaining)"; }
    void clear() override;
    void insert(const Course& course) override;
    const Course* find(const std::string& courseNumber) const override;
    std::size_t size() const override { return size_; }
    std::vector<const Course*> allSorted() const override;

    std::size_t bucketCount() const { return buckets_.size(); }
    double loadFactor() const;
    // Length of the longest chain; a quick health check on the hash spread.
    std::size_t longestChain() const;

private:
    struct Entry {
        std::string key;
        Course course;
    };

    static std::uint64_t hashKey(const std::string& key);
    std::size_t bucketFor(const std::string& key) const;
    void grow();

    std::vector<std::vector<Entry>> buckets_;
    std::size_t size_ = 0;
};

}  // namespace abcu

#endif
