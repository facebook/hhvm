# Copyright (c) Meta Platforms, Inc. and affiliates.
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

# - Try to find librt
# Once done, this will define
#
# LIBRT_FOUND - system has librt
# LIBRT_LIBRARIES - link these to use librt

include(FindPackageHandleStandardArgs)

find_library(LIBRT_LIBRARY rt
  PATHS ${LIBRT_LIBRARYDIR})

find_package_handle_standard_args(librt DEFAULT_MSG LIBRT_LIBRARY)

mark_as_advanced(LIBRT_LIBRARY)

set(LIBRT_LIBRARIES ${LIBRT_LIBRARY})
