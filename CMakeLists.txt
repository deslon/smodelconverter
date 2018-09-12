
cmake_minimum_required (VERSION 2.6)

project (SModelConverter)

set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_TESTS OFF)

include_directories ("${PROJECT_SOURCE_DIR}/ThirdParty/assimp-4.1.0/include")
add_subdirectory (ThirdParty/assimp-4.1.0) 

add_executable(smodelconverter main.cpp WriteSModel.cpp ReadSModel.cpp)

target_link_libraries (smodelconverter assimp)

set_target_properties( smodelconverter
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
