# TODO: Fix libpqxx Docker Build

## Task
Fix CMake build failure for libpqxx-7.7.5 on Alpine Linux

## Root Cause Analysis
The build failure was caused by:
1. PostgreSQL include path not being set correctly on Alpine
2. Config header generation issues with the pre-configured approach
3. CMake warnings being treated as errors
4. C++ template type access issues in lambdas
5. Missing include for util.hxx in binarystring.hxx

## Changes Made

### 1. libpqxx-7.7.5/cmake/config-alpine.cmake
- Added dynamic PostgreSQL header detection for multiple PostgreSQL versions (12-16)
- Added verification that libpq-fe.h exists before proceeding
- Added better error messages for troubleshooting
- Removed trailing whitespace

### 2. Dockerfile
- Added `-DCMAKE_CXX_FLAGS="-Wall -Wno-error"` to prevent warnings from failing the build
- Added error logging to show CMakeError.log on failure
- Removed trailing blank lines

### 3. libpqxx-7.7.5/include/pqxx/internal/conversions.hxx
- Made `elt_type` and `elt_traits` public in `array_string_traits` struct so lambdas can access them

### 4. libpqxx-7.7.5/include/pqxx/binarystring.hxx
- Added `#include "pqxx/util.hxx"` to properly include binary_cast function template

## Status
- [x] Analyze the issue
- [x] Fix config-alpine.cmake with correct PostgreSQL include path detection
- [x] Update Dockerfile with proper build configuration and error handling
- [x] Fix array_string_traits type access in lambdas (make public)
- [x] Fix missing util.hxx include in binarystring.hxx
- [x] Fix root cause: Debian 12 ships libpqxx 6.4.5, but project needs 7.7.5
- [x] Updated Dockerfile to build libpqxx 7.7.5 from local source
- [ ] Verify Docker build works

## How to Test
Run the Docker build:
```bash
cd /home/adminstd/gitpr
docker build -t service-system .
```

If the build still fails, check the error output which will now include the CMakeError.log for debugging.

