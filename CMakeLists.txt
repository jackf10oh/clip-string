cmake_minimum_required(VERSION 3.16)
project(clip_string LANGUAGES CXX)
add_library(clip_string INTERFACE)

# configure ctest to find test added by subdirectories
include(FetchContent)

# set cxx standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# header only library ===============================
target_include_directories(clip_string
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# adding google test
if(BUILD_TESTING)
  enable_testing()
  # find_package(GTest) #find the gtest library  # will fail if not found
  FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG v1.14.0
  )
  FetchContent_MakeAvailable(googletest)
  if(TARGET GTest::gtest)
    message(STATUS "GTest found.") # building tests 
    add_subdirectory(tests)
  else()
    message(WARNING "GTest not found.")
  endif()
endif()

# cmake .. `
#   -G "MinGW Makefiles" `
#   -DCMAKE_C_COMPILER=C:/msys64/mingw64/bin/gcc.exe `
#   -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe `