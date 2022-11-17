include(FetchContent)

FetchContent_Declare(
  json 
  URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
  URL_HASH SHA256=8c4b26bf4b422252e13f332bc5e388ec0ab5c3443d24399acb675e68278d341f
)
FetchContent_MakeAvailable(json)

# Build lib -- this will help with linking with gtest
add_library(cxxenginelib Driver.cpp)
target_include_directories(
  cxxenginelib PRIVATE
  ${CMAKE_SOURCE_DIR}/src
)
target_sources(cxxenginelib PRIVATE
  AnalyserContext.cpp
  CheckFactory.cpp
  CheckInterface.cpp
  ClangTool.cpp
  Driver.cpp
)

target_link_libraries(cxxenginelib nlohmann_json::nlohmann_json)

# Link against LLVM libraries
# Find the libraries that correspond to the LLVM components that we wish to use
# llvm_map_components_to_libnames(llvm_libs support core irreader)
# Imp: Without clang-cpp we crash big; Can we
# achieve this using llvm_map_components_to_libnames maybe
if ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
  target_link_libraries(
    cxxenginelib
    libclang-cpp.dylib
    libLLVM.dylib
  )

elseif ( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
  target_link_libraries(
    cxxenginelib
    libclang-cpp.so
    libLLVM.so
  )

endif()

# Build exec using the above lib
add_executable(cxxengine main.cpp)
target_include_directories(
  cxxengine PRIVATE
  ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(cxxengine cxxenginelib)
