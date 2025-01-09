include(ExternalProject)
include(HPHPFunctions)
# We usually use SHA512, but given the SHA256 is on the boost.org download
# page, use that for transparency/ease of confirmation.
SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
  BOOST_DOWNLOAD_ARGS
  SOURCE_URL
  "https://archives.boost.io/release/1.83.0/source/boost_1_83_0.tar.gz"
  SOURCE_HASH
  "SHA256=c0685b68dd44cc46574cce86c4e17c0f611b15e195be9848dfd0769a0a207628"
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
  cxxstd=${CMAKE_CXX_STANDARD}
)

if (CLANG_FORCE_LIBCPP)
  list(APPEND B2_ARGS toolset=clang stdlib=libc++)
endif()

string(REPLACE ";" "," BOOST_COMPONENTS_CSV "${BOOST_COMPONENTS}")
# We pass --with-libraires to bootstrap.sh, but that does not consistently
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
  CONFIGURE_COMMAND
    ${CMAKE_COMMAND} -E env
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
