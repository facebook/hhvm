# Finds Watchamn C++ client library
#
# This module defines:
# WATCHMANCLIENT_INCLUDE_DIRS
# WATCHMANCLIENT_LIBRARIES
#

find_package(PkgConfig)
pkg_check_modules(WATCHMANCLIENT watchmanclient)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
  libWatchmanClient
  DEFAULT_MSG
  WATCHMANCLIENT_LIBRARIES
  WATCHMANCLIENT_INCLUDE_DIRS)
