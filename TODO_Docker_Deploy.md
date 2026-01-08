# Docker Build Fix - TODO List

## Tasks Completed
- [x] Analyze build errors from libpqxx 7.7.5 and Crow
- [x] Identify root causes: C++ standard mismatch and header file issues
- [x] Fix Crow/include/crow/version.h - move #pragma once to top
- [x] Update Dockerfile - change C++17 to C++20 for libpqxx
- [x] Update CMakeLists.txt - change C++17 to C++20

## Tasks In Progress
- [ ] Rebuild Docker image
- [ ] Verify build succeeds

## Tasks Pending
- [ ] Test application with docker-compose

