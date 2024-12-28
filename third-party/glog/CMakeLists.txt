if(CLANG_FORCE_LIBCPP)
  include(ExternalProject)
  include(HPHPFunctions)

  SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
    GLOG_SOURCE_ARGS
    SOURCE_URL
    "https://github.com/google/glog/archive/refs/tags/v0.5.0.zip"
    SOURCE_HASH
    "SHA512=46669f603279edc05e98153247897c8a7b50ebc9e800132cc8c3eef531687691e616b5210ed9a1dfbb5170ea354b76fb9244b383a8d309bacbfcf2810ec07546"
  )

  set(INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/glog-prefix")
  ExternalProject_add(
    bundled_glog
    ${GLOG_SOURCE_ARGS}
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    -DCMAKE_INSTALL_INCLUDEDIR=include
    -DCMAKE_INSTALL_LIBDIR=lib
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}

    -DBUILD_SHARED_LIBS=Off
    -DBUILD_EXAMPLES=Off
    -DBUILD_TESTING=Off

    -Dgflags_DIR=${gflags_DIR}
  )
  ExternalProject_get_property(bundled_glog INSTALL_DIR)

  add_dependencies(bundled_glog gflags)

  add_library(glog INTERFACE)
  add_dependencies(glog bundled_glog)
  target_include_directories(glog INTERFACE "${INSTALL_DIR}/include")
  target_link_libraries(glog INTERFACE
    "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}glog$<$<CONFIG:Debug>:d>${CMAKE_STATIC_LIBRARY_SUFFIX}"
    gflags
  )

  set(GLOG_INSTALL_DIR "${INSTALL_DIR}" PARENT_SCOPE)
else()
  # google-glog
  find_package(Glog REQUIRED)
  if (GLOG_STATIC)
    add_definitions("-DGOOGLE_GLOG_DLL_DECL=")
  endif()
  include_directories(${GLOG_INCLUDE_DIR})

  add_library(glog INTERFACE)
  add_dependencies(glog glog::glog)
  target_link_libraries(glog INTERFACE ${GLOG_LIBRARIES})
  target_include_directories(glog INTERFACE ${GLOG_INCLUDE_DIRS})
endif()
