# This file exists because, in order to handle the multi-config environment that
# Visual Studio allows, we'd have to modify quite a few of CMake's built-in
# scripts for finding specific libraries. Instead of doing that, we just set the
# required variables here if the /deps/ directory is present.

# We assume in this that, if the directory exists, all libs that are part of
# the package are present.
if (EXISTS "${HPHP_HOME}/deps/")
  message(STATUS "Using default paths for MSVC libs.")

  set(depRoot "${HPHP_HOME}/deps")
  set(incRoot "${depRoot}/include")
  set(libRoot "${depRoot}/lib")

  # First up a few variables to make things configure the first time.

  # Disable a few extensions that we can't currently build because we can't
  # compile the library they depend on.
  set(ENABLE_EXTENSION_MEMCACHE OFF CACHE BOOL "")
  set(ENABLE_EXTENSION_MEMCACHED OFF CACHE BOOL "")
  set(ENABLE_EXTENSION_MYSQL OFF CACHE BOOL "")
  set(ENABLE_EXTENSION_MYSQLI OFF CACHE BOOL "")
  set(ENABLE_EXTENSION_PDO_MYSQL OFF CACHE BOOL "")

  # We need to link against the static library version of boost targetting the static
  # runtime, so set the vars required by default.
  set(Boost_USE_STATIC_LIBS ON CACHE BOOL "")
  set(Boost_USE_STATIC_RUNTIME ON CACHE BOOL "")

  # And now the actual libs. Please keep them alphabetical, and keep the lib
  # values with any system libs first, then the debug version, and finally the
  # release version. Also, when setting the variables, set the include directory
  # first, then any library variables, and finally any misc. variables.

  set(BZIP2_INCLUDE_DIR "${incRoot}" CACHE PATH "")
  set(BZIP2_LIBRARIES "debug;${libRoot}/libbz2MTd.lib;optimized;${libRoot}/libbz2MT.lib" CACHE FILEPATH "")

  set(CURL_INCLUDE_DIR "${incRoot}" CACHE PATH "")
  set(CURL_LIBRARY "Wldap32.lib;debug;${libRoot}/libcurlMTd.lib;optimized;${libRoot}/libcurlMT.lib" CACHE FILEPATH "")
  set(CURL_STATIC ON CACHE BOOL "")

  set(EXPAT_INCLUDE_DIR "${incRoot}" CACHE PATH "")
  set(EXPAT_LIBRARY "debug;${libRoot}/expatMTd.lib;optimized;${libRoot}/expatMT.lib" CACHE FILEPATH "")
  set(EXPAT_STATIC ON CACHE BOOL "")

  set(FREETYPE_INCLUDE_DIRS "${incRoot}" CACHE PATH "")
  set(FREETYPE_LIBRARIES "debug;${libRoot}/libfreetypeMTd.lib;optimized;${libRoot}/libfreetypeMT.lib" CACHE FILEPATH "")

  # Fribidi isn't part of the main package, as the extension isn't supported
  # under MSVC yet, but we'll set the vars here if it's present.
  if (EXISTS "${libRoot}/libfribidiMTd.lib")
    set(FRIBIDI_INCLUDE_DIR "${incRoot}" CACHE PATH "")
    set(FRIBIDI_LIBRARY "debug;${libRoot}/libfribidiMTd.lib;optimized;${libRoot}/libfribidiMT.lib" CACHE FILEPATH "")
  endif()

  set(GMP_INCLUDE_DIR "${incRoot}" CACHE PATH "")
  set(GMP_LIBRARY "debug;${libRoot}/libgmpMTd.lib;optimized;${libRoot}/libgmpMT.lib" CACHE FILEPATH "")

  set(ICU_INCLUDE_DIR "${incRoot}" CACHE PATH "")
  set(ICU_DATA_LIBRARY "debug;${libRoot}/sicudtd.lib;optimized;${libRoot}/sicudt.lib" CACHE FILEPATH "")
  set(ICU_I18N_LIBRARY "debug;${libRoot}/sicuind.lib;optimized;${libRoot}/sicuin.lib" CACHE FILEPATH "")
  set(ICU_LIBRARY "debug;${libRoot}/sicuucd.lib;optimized;${libRoot}/sicuuc.lib" CACHE FILEPATH "")
  set(ICU_STATIC ON CACHE BOOL "")

  set(LIBEVENT_INCLUDE_DIR "${incRoot}" CACHE PATH "")
  set(LIBEVENT_LIB "general;Ws2_32.lib;debug;${libRoot}/eventMTd.lib;debug;${libRoot}/event_coreMTd.lib;debug;${libRoot}/event_extraMTd.lib;optimized;${libRoot}/eventMT.lib;optimized;${libRoot}/event_coreMT.lib;optimized;${libRoot}/event_extraMT.lib" CACHE FILEPATH "")

  set(LIBGLOG_INCLUDE_DIR "${incRoot}" CACHE PATH "")
  set(LIBGLOG_LIBRARY "debug;${libRoot}/libglogMTd.lib;optimized;${libRoot}/libglogMT.lib" CACHE FILEPATH "")
  set(LIBGLOG_STATIC ON CACHE BOOL "")

  set(LIBICONV_INCLUDE_DIR "${incRoot}" CACHE PATH "")
  set(LIBICONV_LIBRARY "debug;${libRoot}/iconvMTd.lib;optimized;${libRoot}/iconvMT.lib" CACHE FILEPATH "")
  set(LIBICONV_CONST ON CACHE BOOL "")

  set(LIBINTL_INCLUDE_DIR "${incRoot}" CACHE PATH "")
  set(LIBINTL_LIBRARY "debug;${libRoot}/libintlMTd.lib;optimized;${libRoot}/libintlMT.lib" CACHE FILEPATH "")

  set(LIBJPEG_INCLUDE_DIRS "${incRoot}" CACHE PATH "")
  set(LIBJPEG_LIBRARIES "debug;${libRoot}/libjpegMTd.lib;optimized;${libRoot}/libjpegMT.lib" CACHE FILEPATH "")

  # LibMagicWand includes a LOT of dependent libraries, 17 here, and another 3
  # that other extensions are dependent on, so are set through those variables.
  # Specifically, Freetype, libJpeg, and libPng.
  set(LIBMAGICKWAND_INCLUDE_DIRS "${incRoot}" CACHE PATH "")
  set(LIBMAGICKCORE_LIBRARIES "debug;${libRoot}/libcairoMTd.lib;debug;${libRoot}/libcodersMTd.lib;debug;${libRoot}/libcrocoMTd.lib;debug;${libRoot}/libexrMTd.lib;debug;${libRoot}/libffiMTd.lib;debug;${libRoot}/libfiltersMTd.lib;debug;${libRoot}/libglibMTd.lib;debug;${libRoot}/libjp2MTd.lib;debug;${libRoot}/liblcmsMTd.lib;debug;${libRoot}/liblqrMTd.lib;debug;${libRoot}/libmagickMTd.lib;debug;${libRoot}/libopenjpegMTd.lib;debug;${libRoot}/libpangoMTd.lib;debug;${libRoot}/libpixmanMTd.lib;debug;${libRoot}/librsvgMTd.lib;debug;${libRoot}/libtiffMTd.lib;debug;${libRoot}/libwebpMTd.lib;optimized;${libRoot}/libcairoMT.lib;optimized;${libRoot}/libcodersMT.lib;optimized;${libRoot}/libcrocoMT.lib;optimized;${libRoot}/libexrMT.lib;optimized;${libRoot}/libffiMT.lib;optimized;${libRoot}/libfiltersMT.lib;optimized;${libRoot}/libglibMT.lib;optimized;${libRoot}/libjp2MT.lib;optimized;${libRoot}/liblcmsMT.lib;optimized;${libRoot}/liblqrMT.lib;optimized;${libRoot}/libmagickMT.lib;optimized;${libRoot}/libopenjpegMT.lib;optimized;${libRoot}/libpangoMT.lib;optimized;${libRoot}/libpixmanMT.lib;optimized;${libRoot}/librsvgMT.lib;optimized;${libRoot}/libtiffMT.lib;optimized;${libRoot}/libwebpMT.lib" CACHE FILEPATH "")
  set(LIBMAGICKWAND_LIBRARIES "debug;${libRoot}/libwandMTd.lib;optimized;${libRoot}/libwandMT.lib" CACHE FILEPATH "")

  set(LIBPNG_INCLUDE_DIRS "${incRoot}" CACHE PATH "")
  set(LIBPNG_LIBRARIES "debug;${libRoot}/libpngMTd.lib;optimized;${libRoot}/libpngMT.lib" CACHE FILEPATH "")

  set(LIBPTHREAD_INCLUDE_DIRS "${incRoot}" CACHE PATH "")
  set(LIBPTHREAD_LIBRARIES "debug;${libRoot}/libpthreadMTd.lib;optimized;${libRoot}/libpthreadMT.lib" CACHE FILEPATH "")
  set(LIBPTHREAD_STATIC ON CACHE BOOL "")

  set(LIBXML2_INCLUDE_DIR "${incRoot}" CACHE PATH "")
  set(LIBXML2_LIBRARIES "debug;${libRoot}/libxml2MTd.lib;optimized;${libRoot}/libxml2MT.lib" CACHE FILEPATH "")
  set(LIBXML2_DEFINITIONS "-DLIBXML_STATIC" CACHE STRING "")

  # Due to being slightly lazy, exslt is built as part of xslt, so both set both
  # variables to the same value, and let CMake get rid of the duplicates.
  set(LIBXSLT_INCLUDE_DIR "${incRoot}" CACHE PATH "")
  set(LIBXSLT_LIBRARIES "debug;${libRoot}/libxsltMTd.lib;optimized;${libRoot}/libxsltMT.lib" CACHE FILEPATH "")
  set(LIBXSLT_EXSLT_LIBRARY "debug;${libRoot}/libxsltMTd.lib;optimized;${libRoot}/libxsltMT.lib" CACHE FILEPATH "")
  set(LIBXSLT_STATIC ON CACHE BOOL "")

  set(Mcrypt_INCLUDE_DIR "${incRoot}" CACHE PATH "")
  set(Mcrypt_LIB "debug;${libRoot}/libmcryptMTd.lib;optimized;${libRoot}/libmcryptMT.lib" CACHE FILEPATH "")

  # Oniguruma is just weird and grumpy, so we set that it's been found explicitly.
  set(ONIGURUMA_FOUND ON CACHE BOOL "" FORCE)
  set(ONIGURUMA_INCLUDE_DIR "${incRoot}" CACHE PATH "")
  set(ONIGURUMA_LIBRARY "debug;${libRoot}/onigMTd.lib;optimized;${libRoot}/onigMT.lib" CACHE FILEPATH "")
  set(ONIGURUMA_STATIC ON CACHE BOOL "")

  set(OPENSSL_INCLUDE_DIR "${incRoot}" CACHE PATH "")
  set(LIB_EAY_DEBUG "${libRoot}/libeay32MTd.lib" CACHE FILEPATH "")
  set(LIB_EAY_RELEASE "${libRoot}/libeay32MT.lib" CACHE FILEPATH "")
  set(SSL_EAY_DEBUG "${libRoot}/ssleay32MTd.lib" CACHE FILEPATH "")
  set(SSL_EAY_RELEASE "${libRoot}/ssleay32MT.lib" CACHE FILEPATH "")

  set(READLINE_INCLUDE_DIR "${incRoot}" CACHE PATH "")
  set(READLINE_LIBRARY "debug;${libRoot}/readlineMTd.lib;optimized;${libRoot}/readlineMT.lib" CACHE FILEPATH "")
  set(READLINE_STATIC ON CACHE BOOL "")

  # TBB's found detection is weird, so we have to set all of these.
  set(TBB_INSTALL_DIR "${depRoot}" CACHE PATH "")
  set(TBB_LIBRARY_DIRS "${depRoot}" CACHE PATH "")
  set(TBB_LIBRARY "${libRoot}/tbb.lib" CACHE FILEPATH "")
  set(TBB_LIBRARY_DEBUG "${libRoot}/tbb_debug.lib" CACHE FILEPATH "")
  set(TBB_MALLOC_LIBRARY "${libRoot}/tbbmalloc.lib" CACHE FILEPATH "")
  set(TBB_MALLOC_LIBRARY_DEBUG "${libRoot}/tbbmalloc_debug.lib" CACHE FILEPATH "")

  set(ZLIB_ROOT "${depRoot}" CACHE PATH "")
  set(ZLIB_LIBRARY "debug;${libRoot}/zlibMTd.lib;optimized;${libRoot}/zlibMT.lib" CACHE FILEPATH "")
endif()
