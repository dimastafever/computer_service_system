# TODO: Fix libpqxx Docker Build

## Task
Fix CMake build failure for libpqxx-7.7.5 on Debian 12

## Issues Fixed
1. [x] Fix result.cxx - Remove buggy multidimensional subscript operator implementation
2. [x] Update Dockerfile - Use C++20 for better compatibility and proper libpq setup
3. [ ] Verify Docker build works

## Plan

### Step 1: Fix result.cxx
Remove the 2-argument `operator[]` implementation that has:
- Typo: `field_num` instead of `col_num`
- Not needed for C++20+ (we're using C++23)
- Preprocessor guard isn't working correctly

### Step 2: Update Dockerfile
- Use C++20 instead of C++23 for better library compatibility
- Ensure libpq-dev is installed for headers
- Ensure libpq5 is in runtime for libpqxx to work

### Step 3: Test Build
Run Docker build to verify fix

