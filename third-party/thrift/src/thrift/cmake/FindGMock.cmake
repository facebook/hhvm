# Copyright (c) Facebook, Inc. and its affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Find GMock
#
# This will define:
# GMOCK_FOUND
# GMOCK_INCLUDE_DIRS
# GMOCK_LIBRARIES
# GMOCK_MAIN_LIBRARIES
# GMOCK_BOTH_LIBRARIES

find_path(GMOCK_INCLUDE_DIRS gmock/gmock.h
    HINTS
        $ENV{GMOCK_ROOT}/include
        ${GMOCK_ROOT}/include
)

find_library(GMOCK_LIBRARIES
    NAMES gmock
    HINTS
        $ENV{GMOCK_ROOT}
        ${GMOCK_ROOT}
)

find_library(GMOCK_MAIN_LIBRARIES
    NAMES gmock_main
    HINTS
        $ENV{GMOCK_ROOT}
        ${GMOCK_ROOT}
)

set(GMOCK_BOTH_LIBRARIES ${GMOCK_LIBRARIES} ${GMOCK_MAIN_LIBRARIES})

mark_as_advanced(GMOCK_INCLUDE_DIRS GMOCK_LIBRARIES GMOCK_MAIN_LIBRARIES)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    GMock GMOCK_LIBRARIES GMOCK_INCLUDE_DIRS GMOCK_MAIN_LIBRARIES)

if(GMOCK_FOUND AND NOT GMOCK_FIND_QUIETLY)
    message(STATUS "GMOCK: ${GMOCK_INCLUDE_DIRS}")
endif()
