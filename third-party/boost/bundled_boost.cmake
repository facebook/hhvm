include(ExternalProject)                                                        
include(HPHPFunctions)
# We usually use SHA512, but given the SHA256 is on the boost.org download
# page, use that for transparency/ease of confirmation.
SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
  BOOST_DOWNLOAD_ARGS
  SOURCE_URL
  "https://boostorg.jfrog.io/artifactory/main/release/1.73.0/source/boost_1_73_0.tar.bz2"
  SOURCE_HASH
  "SHA256=4eb3b8d442b426dc35346235c8733b5ae35ba431690e38c6a8263dce9fcbb402"
)
set(
  COMMON_ARGS
  --prefix=<INSTALL_DIR>
  --libdir=<INSTALL_DIR>/lib
)
set(
  B2_ARGS
  ${COMMON_ARGS}
  --build-dir=<BINARY_DIR>
  link=static
  variant=release
  threading=multi
  runtime-link=static
  cxxflags=-std=gnu++14
)

string(REPLACE ";" "," BOOST_COMPONENTS_CSV "${BOOST_COMPONENTS}")
# We pass --with-libraires to boostrap.sh, but that does not consistently
# affect b2
foreach(COMPONENT IN LISTS BOOST_COMPONENTS)
  list(APPEND B2_ARGS "--with-${COMPONENT}")
endforeach()

if (APPLE)
  set(BOOST_CXX_FLAGS "-isysroot${CMAKE_OSX_SYSROOT}")
endif()

ExternalProject_Add(                                                            
  bundled_boost
  ${BOOST_DOWNLOAD_ARGS}
  PATCH_COMMAND
    cd tools/build && patch -p1 < "${CMAKE_CURRENT_SOURCE_DIR}/b3a59d265929a213f02a451bb6-macos-coalesce-template.patch"
  CONFIGURE_COMMAND
    CXX=${CMAKE_CXX_COMPILER}
    CXXFLAGS=${BOOST_CXX_FLAGS}
    NO_CXX11_CHECK=true # we have c++17 (at least), and the check ignores CXXFLAGS, including `-isysroot` on macos
    <SOURCE_DIR>/bootstrap.sh
    ${COMMON_ARGS}
    "--with-libraries=${BOOST_COMPONENTS_CSV}"
  BUILD_COMMAND
    cd <SOURCE_DIR> && <BINARY_DIR>/b2 ${B2_ARGS}
  INSTALL_COMMAND
    cd <SOURCE_DIR> && <BINARY_DIR>/b2 ${B2_ARGS} install
)
add_dependencies(boost bundled_boost)
ExternalProject_Get_Property(bundled_boost INSTALL_DIR)
target_include_directories(boost INTERFACE "${INSTALL_DIR}/include")
list(REMOVE_ITEM BOOST_COMPONENTS headers)
foreach(COMPONENT IN LISTS BOOST_COMPONENTS)
  target_link_libraries(
    boost
    INTERFACE
    "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}boost_${COMPONENT}${CMAKE_STATIC_LIBRARY_SUFFIX}"
  )
endforeach()
