# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/freetype-src"
  "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/freetype-build"
  "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/freetype-subbuild/freetype-populate-prefix"
  "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/freetype-subbuild/freetype-populate-prefix/tmp"
  "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/freetype-subbuild/freetype-populate-prefix/src/freetype-populate-stamp"
  "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/freetype-subbuild/freetype-populate-prefix/src"
  "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/freetype-subbuild/freetype-populate-prefix/src/freetype-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/freetype-subbuild/freetype-populate-prefix/src/freetype-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/michael/Documents/YURRGOHT_ENGINE/build/_deps/freetype-subbuild/freetype-populate-prefix/src/freetype-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
