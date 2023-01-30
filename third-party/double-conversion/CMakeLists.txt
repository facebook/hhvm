include(ExternalProject)
include(HPHPFunctions)

SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
  DOUBLE_CONVERSION_DOWNLOAD_ARGS
  SOURCE_URL "https://github.com/google/double-conversion/archive/refs/tags/v3.0.1.tar.gz"
  SOURCE_HASH "SHA256=0777bb2702ebf18a16dd3bc2eb0b0e7b3aa1c9b4375feaa87ea4cf9d27be4536"
  # The tarball name is just the tag name, which can conflict in the cache
  FILENAME_PREFIX "double-conversion-"
)

ExternalProject_Add(
  bundled_double-conversion
  ${DOUBLE_CONVERSION_DOWNLOAD_ARGS}
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    -DCMAKE_INSTALL_LIBDIR=lib
    -DCMAKE_INSTALL_INCLUDEDIR=include
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
  EXCLUDE_FROM_ALL
)

option(FORCE_BUNDLED_DOUBLE_CONVERSION "Always use bundled double-conversion" OFF)
if(FORCE_BUNDLED_DOUBLE_CONVERSION)
  set(DOUBLE_CONVERSION_FOUND FALSE)
else()
  find_package(DoubleConversion)
endif()

add_library(double-conversion INTERFACE)
if(DOUBLE_CONVERSION_FOUND)
  set_target_properties(
    double-conversion
    PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${DOUBLE_CONVERSION_INCLUDE_DIR}"
  )
  target_link_libraries(double-conversion INTERFACE "${DOUBLE_CONVERSION_LIBRARY}")
else()
  add_dependencies(double-conversion bundled_double-conversion)
  ExternalProject_Get_Property(bundled_double-conversion INSTALL_DIR)
  set_target_properties(
    double-conversion
    PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${INSTALL_DIR}/include"
  )
  target_link_libraries(
    double-conversion
    INTERFACE
    "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}double-conversion${CMAKE_STATIC_LIBRARY_SUFFIX}"
  )
endif()
