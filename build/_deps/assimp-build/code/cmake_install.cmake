# Install script for directory: /home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "libassimp5.4.3" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libassimpd.so.5.4.3"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libassimpd.so.5"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-build/bin/libassimpd.so.5.4.3"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-build/bin/libassimpd.so.5"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libassimpd.so.5.4.3"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libassimpd.so.5"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "libassimp5.4.3" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libassimpd.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libassimpd.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libassimpd.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-build/bin/libassimpd.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libassimpd.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libassimpd.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libassimpd.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "assimp-dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/assimp" TYPE FILE FILES
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/anim.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/aabb.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/ai_assert.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/camera.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/color4.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/color4.inl"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-build/code/../include/assimp/config.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/ColladaMetaData.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/commonMetaData.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/defs.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/cfileio.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/light.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/material.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/material.inl"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/matrix3x3.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/matrix3x3.inl"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/matrix4x4.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/matrix4x4.inl"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/mesh.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/ObjMaterial.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/pbrmaterial.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/GltfMaterial.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/postprocess.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/quaternion.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/quaternion.inl"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/scene.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/metadata.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/texture.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/types.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/vector2.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/vector2.inl"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/vector3.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/vector3.inl"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/version.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/cimport.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/AssertHandler.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/importerdesc.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/Importer.hpp"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/DefaultLogger.hpp"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/ProgressHandler.hpp"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/IOStream.hpp"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/IOSystem.hpp"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/Logger.hpp"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/LogStream.hpp"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/NullLogger.hpp"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/cexport.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/Exporter.hpp"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/DefaultIOStream.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/DefaultIOSystem.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/ZipArchiveIOSystem.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/SceneCombiner.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/fast_atof.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/qnan.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/BaseImporter.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/Hash.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/MemoryIOWrapper.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/ParsingUtils.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/StreamReader.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/StreamWriter.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/StringComparison.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/StringUtils.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/SGSpatialSort.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/GenericProperty.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/SpatialSort.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/SkeletonMeshBuilder.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/SmallVector.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/SmoothingGroups.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/SmoothingGroups.inl"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/StandardShapes.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/RemoveComments.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/Subdivision.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/Vertex.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/LineSplitter.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/TinyFormatter.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/Profiler.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/LogAux.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/Bitmap.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/XMLTools.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/IOStreamBuffer.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/CreateAnimMesh.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/XmlParser.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/BlobIOSystem.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/MathFunctions.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/Exceptional.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/ByteSwapper.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/Base64.hpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "assimp-dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/assimp/Compiler" TYPE FILE FILES
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/Compiler/pushpack1.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/Compiler/poppack1.h"
    "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/assimp-src/code/../include/assimp/Compiler/pstdint.h"
    )
endif()

