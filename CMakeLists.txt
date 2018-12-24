
cmake_minimum_required (VERSION 3.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

set( CMAKE_CXX_FLAGS " -pthread " )

project (SModelConverter)

set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_TESTS OFF)

include_directories ("${PROJECT_SOURCE_DIR}/ThirdParty/assimp-4.1.0/include")
add_subdirectory (ThirdParty/assimp-4.1.0) 

include_directories ("${PROJECT_SOURCE_DIR}/ThirdParty/stb")
include_directories ("${PROJECT_SOURCE_DIR}/ThirdParty/json")
include_directories ("${PROJECT_SOURCE_DIR}/ThirdParty/fifo_map")

# FBX
foreach (FBXSDK_VERSION "2019.0" "2018.1.1")
  find_package(FBX)
  if (FBXSDK_FOUND)
    break()
  endif()
endforeach(FBXSDK_VERSION)
if (NOT FBXSDK_FOUND)
  message(FATAL_ERROR
    "Can't find FBX SDK in either:\n"
    " - Mac OS X: ${FBXSDK_APPLE_ROOT}\n"
    " - Windows: ${FBXSDK_WINDOWS_ROOT}\n"
    " - Linux: ${FBXSDK_LINUX_ROOT}"
  )
endif()

add_executable(smodelconverter 
  main.cpp
  Assimp2SModel.cpp
  Fbx2SModel.cpp
  WriteSModel.cpp
  ReadSModel.cpp
  util/Fbx2Raw.cpp
  util/FbxBlendShapesAccess.cpp
  util/FbxMaterialsAccess.cpp
  util/FbxSkinningAccess.cpp
  util/FbxTraditionalMaterialInfo.cpp
  util/FbxRoughMetMaterialInfo.cpp
  util/RawModel.cpp
  util/File_Utils.cpp
  util/String_Utils.cpp
  util/Image_Utils.cpp)

target_link_libraries (smodelconverter 
  assimp
  optimized ${FBXSDK_LIBRARY}
  debug ${FBXSDK_LIBRARY_DEBUG}
  ${CMAKE_DL_LIBS}
  ${CMAKE_THREAD_LIBS_INIT}
  )

target_include_directories(smodelconverter PUBLIC
  ${FBXSDK_INCLUDE_DIR}
)

set_target_properties( smodelconverter
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
