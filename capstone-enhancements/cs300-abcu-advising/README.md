# ABCU Advising Program (CS-499 Milestone Three - Algorithms & Data Structures)

Enhances the CS-300 Project Two console advising program
(`coursework/CS-300_Data-Structures-and-Algorithms/CS300_Project_Two_Heath_Davis.cpp`),
a C++ tool that loads the ABCU course catalog from a CSV file into a `std::map`
and lets the user list courses and look one up.

The enhancements follow the Module 1 Enhancement Plan and the specific
commitments made on camera in the Module 2 code-review video
(`Downloads\CS-499\CS499_Milestone1_Video_Outline.md`, Category 1 section). The
original file is untouched; this directory is the enhanced version.

## What changed and why

| Commitment (Module 2 review) | Original | Enhanced |
|---|---|---|
| Hash table plus a measured comparison | One structure, `std::map`, and an unmeasured claim that O(log n) was the right choice | `HashCourseStore`, a separate-chaining hash table written from scratch (FNV-1a, power-of-two buckets, doubles at load factor 0.75), behind a `CourseStore` interface shared with the original ordered map. `abcu_benchmark` times both on the real catalog and on 113,933 real keys. Results in `benchmark/results.txt` |
| Prerequisites as a graph: cycle detection and topological sort | Prerequisites were plain strings. A circular chain, or a prerequisite that is not in the catalog, printed without complaint | `PrerequisiteGraph`: iterative three-colour DFS finds a circular chain and reports the actual loop; missing prerequisites are reported by course; Kahn's algorithm produces a study order (menu option 5). Loading a catalog now warns when the data has problems |
| Unit tests | None. Correctness was eyeballed from console output | 34 tests (`tests/test_main.cpp`) covering parsing, both data structures, the graph, and the menu. Fixtures come from the real catalog; a broken catalog is made by editing one real line. A deliberate-breakage check (below) confirmed the tests fail when the code is wrong |
| Replace magic menu numbers, handle bad input | `case 1`, `case 2`, `case 9` literals. `cin >> choice` on a letter left `cin` failed forever and the loop printed the invalid-option message endlessly | `enum class MenuOption`. Every prompt reads a whole line and validates it; input that ends (EOF) leaves the loop instead of spinning |
| Replace the hand-rolled CSV split with the existing reusable parser | A private `split()` function in the same file | `csv::Parser` (`third_party/csvparser`) behind `loadCatalogFromText` / `loadCatalogFromFile` |

Also changed while in the code: the global `courseMap` is now owned by the app
instead of a file-level global; file and course-number prompts read a full line
so a path with spaces works; loading counts distinct courses (the original
counted a repeated course number twice) and warns about duplicates; a row with
no course name is reported instead of silently skipped.

### Two deviations from the on-camera wording

1. **The CSV parser is not mine.** The review said the reusable parser was "already
   built once before in my own coursework." It is the `CSVparser` library the
   course supplied with the Binary Search Tree, Hash Table, and Vector Sorting
   assignments (MIT licensed, copyright Romain Sylvian; licence kept in
   `third_party/csvparser/License.txt`). I reused it, but I did not write it, and
   this README should not imply otherwise. It also does not fit the catalog file
   directly: it treats line 1 as a header and throws if any row has a different
   column count, while the catalog has no header. `catalog_loader.cpp` adapts it
   by prepending a header and padding short rows, and says so in a comment.
2. **The "real CSV" is only 8 rows.** The review said I would benchmark on the
   real CSV. I do, but 8 courses cannot show an O(log n) versus O(1) difference.
   So the scaling run uses real data from my own MTG Sorter project: 113,933
   distinct set-code-and-collector-number keys (for example `blb-280`) taken from
   Scryfall bulk data, which have the same letters-then-digits shape as course
   numbers. No generated data was used. That file is not committed (about 3 MB);
   `benchmark/extract_scryfall_keys.py` rebuilds it from any SQLite database with
   the same `printings` table.

## Complexity

| Operation | Ordered map | Hash table |
|---|---|---|
| insert / find | O(log n) | O(1) average, O(n) worst case |
| list in course-number order | O(n), already ordered | O(n log n), collect then sort |
| validate prerequisites (DFS) | O(V + E) | O(V + E) |
| study order (Kahn, min-heap) | O((V + E) log V) | O((V + E) log V) |

## Measured results

From `benchmark/results.txt` (Intel Pentium Silver N5030 at 1.1 GHz, Release
build, fastest of 5 repeats, one recorded run; absolute numbers move between
runs on this laptop, the ratios are what to read):

| Workload | Ordered map find (hit) | Hash table find (hit) | Ordered map list | Hash table list |
|---|---|---|---|---|
| Real ABCU catalog, 8 courses | 227 ns | 133 ns | not timed | not timed |
| 1,000 real keys | 458 ns | 166 ns | 28 ns/course | 442 ns/course |
| 113,933 real keys | 1,765 ns | 578 ns | 170 ns/course | 1,739 ns/course |

- At full size the hash table finds a key about 3x faster than the map, and the
  map lists about 10x faster than the hash table, because the map is already
  ordered. Which is "better" depends on which operation the program does more of.
  An advising tool that mostly looks one course up favours the hash table; one
  that prints the sorted catalog often favours the map. The application keeps
  the map for that reason, and the hash table is available behind the same
  interface.
- Loading is a near tie at full size (2,317 ns versus 2,124 ns per course) and
  the map is faster at small sizes (740 versus 1,235 ns at 1,000 keys). I did not
  profile why; the likely cause is the hash table's rehashing and per-bucket
  allocation as it grows.
- The hash table's lookup time is not flat. It rose from 166 ns to 578 ns across
  that range. I did not profile it either; the likely cause is cache behaviour on
  this machine (the bucket array outgrows the cache), not a failure of the O(1)
  average case. The measured result is a
  smaller slope than the map's (3.5x versus 3.9x over a 114x size increase, and
  roughly one third the absolute time), not a constant.

## Build, run, test

Needs a C++17 compiler and CMake 3.16 or newer (tested with MSVC 19.41 and the
CMake that ships with Visual Studio 2022).

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/abcu_advising                 # the interactive program
./build/abcu_tests                    # or: ctest --test-dir build
./build/abcu_benchmark                # real catalog only
./build/abcu_benchmark --keys scryfall_keys.tsv   # plus the scaling run
```

Enter `data/ABCU_Advising_Program_Input.csv` when the program asks for a
filename.

## Verification

- `abcu_tests`: 34 tests, 0 failed checks on the delivered code.
- Deliberate-breakage check: with cycle detection disabled and with hash-table
  growth disabled, 13 checks across 4 tests failed; the originals were restored
  and the suite passes again. The tests are capable of failing.
- Regression for the original bug: compiling the original file and giving it the
  input `abc` printed "0 is not a valid option." 1,144 times in 3 seconds and
  never exited. The enhanced program rejects the line and reaches the next
  prompt; `menu_non_numeric_input_is_rejected_and_the_loop_still_exits` locks that in.

## Layout

```
src/            course, the two stores, loader, graph, menu app, main
tests/          test_main.cpp (no framework)
benchmark/      benchmark.cpp, extract_scryfall_keys.py, results.txt
data/           the real ABCU catalog
third_party/    csvparser (course-supplied, MIT)
```
