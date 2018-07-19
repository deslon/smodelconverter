
cmake_minimum_required (VERSION 2.6)

project (SModelConv)

set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(BUILD_SHARED_LIBS OFF)

include_directories ("${PROJECT_SOURCE_DIR}/ThirdParty/assimp-4.1.0/include")
add_subdirectory (ThirdParty/assimp-4.1.0) 

add_executable(smodelconv main.cpp)

target_link_libraries (smodelconv assimp)

set_target_properties( smodelconv
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)