# CMake Build Fix Plan

## Issue
CMake cannot find libpqxx headers. Error:
```
Could not find PQXX_INCLUDE_DIR using the following files: pqxx/pqxx
```

## Root Cause
The libpqxx source directory is copied but not built/installed in the Dockerfile's builder stage. The main CMakeLists.txt expects headers in /usr/include or /usr/local/include, but they aren't there.

## Fix Plan

### Step 1: Update Dockerfile
- Build libpqxx library first before building the main application
- Install libpqxx to /usr/local so CMake can find it
- Add cmake files from libpqxx to CMake module path

### Step 2: Update CMakeLists.txt
- Add /usr/local/include to PQXX_INCLUDE_DIR search paths
- Ensure proper linking to installed libpqxx library

## Implementation Status
- [x] Update Dockerfile to build and install libpqxx
- [x] Update CMakeLists.txt to search /usr/local/include
- [ ] Test the build

