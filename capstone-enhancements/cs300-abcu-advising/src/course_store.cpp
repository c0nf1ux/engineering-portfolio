#include "course_store.hpp"

#include <algorithm>
#include <cstdint>

namespace abcu {

// ---- OrderedCourseStore ----------------------------------------------------

void OrderedCourseStore::insert(const Course& course) {
    courses_[normalizeKey(course.courseNumber)] = course;
}

const Course* OrderedCourseStore::find(const std::string& courseNumber) const {
    auto it = courses_.find(normalizeKey(courseNumber));
    return it == courses_.end() ? nullptr : &it->second;
}

std::vector<const Course*> OrderedCourseStore::allSorted() const {
    std::vector<const Course*> result;
    result.reserve(courses_.size());
    for (const auto& entry : courses_) {
        result.push_back(&entry.second);
    }
    return result;
}

// ---- HashCourseStore -------------------------------------------------------

namespace {
constexpr double kMaxLoadFactor = 0.75;
constexpr std::uint64_t kFnvOffsetBasis = 14695981039346656037ULL;
constexpr std::uint64_t kFnvPrime = 1099511628211ULL;

// Bucket counts stay powers of two so the index is a mask, not a modulo.
std::size_t roundUpToPowerOfTwo(std::size_t value) {
    std::size_t result = 1;
    while (result < value) {
        result <<= 1;
    }
    return result;
}
}  // namespace

HashCourseStore::HashCourseStore(std::size_t initialBuckets)
    : buckets_(roundUpToPowerOfTwo(std::max<std::size_t>(initialBuckets, 1))) {}

std::uint64_t HashCourseStore::hashKey(const std::string& key) {
    std::uint64_t hash = kFnvOffsetBasis;
    for (unsigned char c : key) {
        hash ^= c;
        hash *= kFnvPrime;
    }
    return hash;
}

std::size_t HashCourseStore::bucketFor(const std::string& key) const {
    return static_cast<std::size_t>(hashKey(key)) & (buckets_.size() - 1);
}

void HashCourseStore::clear() {
    for (auto& bucket : buckets_) {
        bucket.clear();
    }
    size_ = 0;
}

void HashCourseStore::insert(const Course& course) {
    const std::string key = normalizeKey(course.courseNumber);
    auto& chain = buckets_[bucketFor(key)];
    for (auto& entry : chain) {
        if (entry.key == key) {
            entry.course = course;
            return;
        }
    }
    chain.push_back(Entry{key, course});
    ++size_;
    if (loadFactor() > kMaxLoadFactor) {
        grow();
    }
}

const Course* HashCourseStore::find(const std::string& courseNumber) const {
    const std::string key = normalizeKey(courseNumber);
    for (const auto& entry : buckets_[bucketFor(key)]) {
        if (entry.key == key) {
            return &entry.course;
        }
    }
    return nullptr;
}

void HashCourseStore::grow() {
    std::vector<std::vector<Entry>> old;
    old.swap(buckets_);
    buckets_.assign(old.size() * 2, std::vector<Entry>());
    for (auto& chain : old) {
        for (auto& entry : chain) {
            buckets_[bucketFor(entry.key)].push_back(std::move(entry));
        }
    }
}

double HashCourseStore::loadFactor() const {
    return static_cast<double>(size_) / static_cast<double>(buckets_.size());
}

std::size_t HashCourseStore::longestChain() const {
    std::size_t longest = 0;
    for (const auto& chain : buckets_) {
        longest = std::max(longest, chain.size());
    }
    return longest;
}

std::vector<const Course*> HashCourseStore::allSorted() const {
    std::vector<const Entry*> entries;
    entries.reserve(size_);
    for (const auto& chain : buckets_) {
        for (const auto& entry : chain) {
            entries.push_back(&entry);
        }
    }
    std::sort(entries.begin(), entries.end(),
              [](const Entry* a, const Entry* b) { return a->key < b->key; });
    std::vector<const Course*> result;
    result.reserve(entries.size());
    for (const Entry* entry : entries) {
        result.push_back(&entry->course);
    }
    return result;
}

}  // namespace abcu
