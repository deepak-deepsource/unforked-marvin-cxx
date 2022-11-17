cmake_minimum_required(VERSION 3.13.4)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Release)
project(CxxEngine)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# add llvm include dir (from llvm-config --cxxflags)
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/src)
message(STATUS "LLVM include dir is ${LLVM_INCLUDE_DIRS}")
separate_arguments(
  LLVM_DEFINITIONS_LIST NATIVE_COMMAND
  ${LLVM_DEFINITIONS}
)
message(STATUS "Adding LLVM preprocessor macros ${LLVM_DEFINITIONS_LIST}")
add_definitions(${LLVM_DEFINITIONS_LIST})

# We do this to find -lclang-cpp or -lllvm-{ver}. This should be solvable using
# find_package and target_link_libraries. But doesn't seem to work at this moment
link_directories(${LLVM_INSTALL_DIR}/lib)

# CXX and LD flags are from llvm-config --cxxflags --ldflags;
# It is weird that we have manually add these though we are using find_package
# to get LLVM config!
# add sub part from llvm-config --cxxflags

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")

if ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
set(
  CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -stdlib=libc++ -fno-rtti"
)
# add llvm-config --ldflags
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -Wl,-search_paths_first -Wl,-headerpad_max_install_names"
)
elseif ( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
# add llvm-config --ldflags
set(
  CMAKE_EXE_LINKER_FLAGS
  "${CMAKE_EXE_LINKER_FLAGS} -L${LLVM_INSTALL_DIR}/lib"
)
endif()

add_subdirectory(src)

if(CMAKE_PROJECT_NAME STREQUAL CxxEngine AND BUILD_TESTING)
  enable_testing() 
  add_subdirectory(test)
endif()
