add_executable(tests_clipstring
  tests_clipstring.cpp
)

target_link_libraries(tests_clipstring
  PRIVATE clip_string
  PRIVATE GTest::gmock_main 
  # PRIVATE pthread # required by GTest # removed for MSVC to compile.
)

include(GoogleTest)
gtest_discover_tests(tests_clipstring)
