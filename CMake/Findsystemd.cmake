find_package(PkgConfig)
pkg_check_modules(SYSTEMD libsystemd)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
  systemd
  DEFAULT_MSG
  SYSTEMD_LIBRARIES
  SYSTEMD_INCLUDEDIR)
  