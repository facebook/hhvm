  include(ExternalProject)
  include(HPHPFunctions)

  SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
    GLOG_SOURCE_ARGS
    SOURCE_URL
    "https://github.com/Neargye/magic_enum/archive/refs/tags/v0.9.7.zip"
    SOURCE_HASH
    "SHA512=99a434597a8cd83e432e6613b3107b7a56882d6cd33a0a060789f5d4e700d47b4cc7ae0a18984f14d7611a1e550361a2f14ee9152ca28254da658558c095a911"
  )


  set(INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/magic_enum-prefix")
  ExternalProject_add(
    bundled_magic_enum
    ${GLOG_SOURCE_ARGS}
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}

    -DMAGIC_ENUM_OPT_BUILD_EXAMPLES=Off
    -DMAGIC_ENUM_OPT_BUILD_TESTS=Off
  )
  ExternalProject_get_property(bundled_magic_enum INSTALL_DIR)

  add_library(magic_enum INTERFACE)
  add_dependencies(magic_enum bundled_magic_enum)
  target_include_directories(magic_enum INTERFACE "${INSTALL_DIR}/include")
