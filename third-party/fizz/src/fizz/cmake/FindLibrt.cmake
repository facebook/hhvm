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
