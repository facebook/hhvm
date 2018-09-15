set(HPHP_HOME "${CMAKE_CURRENT_SOURCE_DIR}/../../..")

include_directories(
  ./
)

HHVM_EXTENSION(dso_test     dso_test.cpp)
HHVM_SYSTEMLIB(dso_test ext_dso_test.php)
