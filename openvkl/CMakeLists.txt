## ======================================================================== ##
## Copyright 2018 Intel Corporation                                         ##
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

## Configure OpenVKL library build ##
include(ispc)

openvkl_add_library_ispc(openvkl SHARED
  api/API.cpp
  api/Driver.cpp

  common/Data.cpp
  common/ispc_util.ispc
  common/logging.cpp
  common/ManagedObject.cpp
  common/VKLCommon.cpp
)

target_include_directories(openvkl
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

## Include ospcommon dependency ##
# TODO: ospray linked here to fix issue with ospcommon include paths
target_link_libraries(openvkl
  PUBLIC
    ospray::ospray
    ospray::ospray_common
)

## Configure OpenVKL installation ##
install(TARGETS openvkl
  EXPORT openvklConfig
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT openvklConfig DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/openvkl)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/openvkl/include/openvkl
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

## Add OpenVKL drivers ##
add_subdirectory(drivers)