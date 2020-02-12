## ======================================================================== ##
## Copyright 2019 Intel Corporation                                         ##
##                                                                          ##
## Licensed under the Apache License, Version 2.0 (the "License");          ##
## you may not use this file except in compliance with the License.         ##
## You may obtain a copy of the License at                                  ##
##                                                                          ##
##     http://www.apache.org/licenses/LICENSE-2.0                           ##
##                                                                          ##
## Unless required by applicable law or agreed to in writing, software      ##
## distributed under the License is distributed on an "AS IS" BASIS,        ##
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. ##
## See the License for the specific language governing permissions and      ##
## limitations under the License.                                           ##
## ======================================================================== ##

find_package(Boost REQUIRED COMPONENTS system filesystem)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

find_package(imebra REQUIRED)
include_directories(${imebra_INCLUDE_DIRS})
link_directories(${imebra_LIBRARY_DIRS})

add_library(openvkl_testing INTERFACE)

add_subdirectory(volume/dcm)

find_package(X11 REQUIRED)
include_directories(${X11_INCLUDE_DIR})

target_link_libraries(openvkl_testing INTERFACE
  openvkl
  ospcommon::ospcommon
  ${Boost_LIBRARIES}
  stdc++fs
  dcm
  imebra
  ${X11_LIBRARIES}
)

target_include_directories(openvkl_testing
  INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if (BUILD_BENCHMARKS)
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE) # don't build tests
  add_subdirectory(external/benchmark)
endif()

# add_subdirectory(apps)
