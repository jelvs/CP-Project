# Concurrency and Parallelism 2018-19

**master** &nbsp;&nbsp; [![CircleCI](https://circleci.com/gh/jelvs/CP-Project/tree/master.svg?style=svg&circle-token=66b0c6a8f10186f9958444d5c6e045e6df10840c)](https://circleci.com/gh/jelvs/CP-Project/tree/master)

**develop** &nbsp;&nbsp; [![CircleCI](https://circleci.com/gh/jelvs/CP-Project/tree/develop.svg?style=svg&circle-token=66b0c6a8f10186f9958444d5c6e045e6df10840c)](https://circleci.com/gh/jelvs/CP-Project/tree/develop)

## Project — Parallel Patterns (v1.0)
---

## Comparing times

Time comparison between intial serial version and parallel versions, are rolling on the wiki.

* [Serial Version](https://github.com/jelvs/CP-Project/wiki/Initial-Code-Times)
* [Parallel Version](https://github.com/jelvs/CP-Project/wiki/Parallel-Version-Times)

---
## Project Description

In this lab work you are given a working version of the projet. It will compile and run and produce results, although the execution will be slow as all the given implementations for the patterns are sequential. You are asked to study the given code a create parallel versions of the patterns using Cilk+.

### 1. Given Version

You are given a (almost working) version of the project at https://bitbucket.org/cp201819/project_parallel_patterns.git
This repository contains two directories/folders: srcanddoc. The former contains the base
source code, and the latter will contain your Project Report as a PDF file. Please name your report
asreport_AAAAA_BBBBB_CCCCC.pdf, whereAAAAA,BBBBBandCCCCCare the numbers of the group
members sorted in increasing order (lowest to highest).

Fork the above repository and name your groups repository as
cp2018-19_project_AAAAA_BBBBB_CCCCC.pdf
whereAAAAA,BBBBBandCCCCCare the numbers of the group members sorted in increasing
order (lowest to highest).

In your Linux device (own laptop, lab workstation, or as a last resort, in the “node9” server
used in the last lab class) clone your new repository and then try compile your code using the
commandmakein thesrcdirectory. It must compile with no errors nor warnings.

### 2. Code Structure

```
debug.c 
debug.h 
main.c
patterns.c
patterns.h
unit.c
unit.h
```

The project include the following source files:

Files Description
* **debug.c debug.h**: Functions for printing the contents of the array(s), useful for debugging (activated with the option “-d”).
* **patterns.c patterns.h**: The patterns to be implemented. The “.c” file contains empty functions.
* **unit.c unit.h**: Functions for unit testing of each pattern.
* **main.c**: The main program.

### 3. Work plan

Your job is to make an optimized parallel version (using Cilk+) of all the patterns listed in the files
patterns.c/patterns.h!.

**You may follow these steps:**
1. Clone/fork the given project.
2. Compile the given version. Study the source code and understand how it works.
3. Discuss with your colleagues how to split the work.
4. Implement a sequential version of each pattern.
5. Compile and run the tests and confirm the results.
6. Implement a parallel version of each pattern.
7. Compile and run the tests and confirm the results.
8. For each pattern, measure its perfocrmance/scalability/scaled scalability. You may experiment with different numbers of processors (by setting the environment variableCILK_NWORKERS).
9. Optimize the given code.
10. Go back to item 7. until satisfied.
11. Write the report. Revise the report. Please put your report in thedocdirectory and name it
asreport_AAAAA_BBBBB_CCCCC.pdf, whereAAAAA,BBBBBandCCCCCare the numbers of the
group members sorted in increasing order (lowest to highest).
12. _Optional:_ implement and optimize some more parallel patterns.
13. _Optional:_ implement and share some more tests (unit or integration tests).
14. _Optional:_ complete the report and revise again.

**Please remember to commit regularly your changes, and please always write mean-
ingful commit messages.**

## 4. Questions/Discussion

Please ask your questions using the Piazza system. Either public (if possible) or private (if really
necessary).

## 5. Delivery Dates
TBD;

## 6. Delivery Method
TBD;


