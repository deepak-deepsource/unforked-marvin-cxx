include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# TODO; This isn't very pretty! We have already included it in top-level CMakeList
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(
  LLVM_DEFINITIONS_LIST NATIVE_COMMAND
  ${LLVM_DEFINITIONS}
)
add_definitions(${LLVM_DEFINITIONS_LIST})

set(
  CMAKE_CXX_FLAGS 
  "${CMAKE_CXX_FLAGS} -stdlib=libc++ -fno-exceptions -fno-rtti"
)
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -Wl,-search_paths_first -Wl,-headerpad_max_install_names"
)

link_directories(${LLVM_INSTALL_DIR}/lib)

add_executable(
  test_cxxengine
  TestIssue.cpp
  TestAnalyserContext.cpp
)

target_include_directories(
  test_cxxengine PRIVATE 
  ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(
  test_cxxengine
  GTest::gtest_main
  libclang-cpp.dylib
  cxxenginelib
)

include(GoogleTest)
gtest_discover_tests(test_cxxengine)
