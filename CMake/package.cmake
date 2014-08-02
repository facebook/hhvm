if(CPACK_GENERATOR)
  # common package information
  set(CPACK_PACKAGE_NAME "${PACKAGE_NAME}")
  set(CPACK_PACKAGE_VENDOR "Facebook")
  set(CPACK_PACKAGE_VERSION "${PACKAGE_VERSION}")
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Virtual Machine, Runtime, and JIT for PHP")
  set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
  #set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_CURRENT_LIST_DIR}/README.md")
  #set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_LIST_DIR}/LICENSE")
  set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_LIST_DIR}/README.md")
  set(CPACK_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
  set(CPACK_PACKAGE_CONTACT "Paul Tarjan <pt@fb.com>")
  set(CPACK_OUTPUT_FILE_PREFIX packages)
  set(CPACK_PACKAGE_RELOCATABLE true)
  set(CPACK_MONOLITHIC_INSTALL true)

  # Prefix Debug/Nightly release
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}")
  if(NIGHTLY)
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-nightly")
    #execute_process(COMMAND "date +%Y.%m.%d" OUTPUT_VARIABLE NIGHTLY_DATE)
  endif()
  if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-dbg")
  endif()

  # default package generators
  if(APPLE)
    set(PACKAGE_GENERATOR "PackageMaker")
    set(PACKAGE_SOURCE_GENERATOR "TGZ;ZIP")
  elseif(UNIX)
    set(PACKAGE_GENERATOR "DEB;RPM")
    set(PACKAGE_SOURCE_GENERATOR "TGZ;ZIP")
  else()
    set(PACKAGE_GENERATOR "ZIP")
    set(PACKAGE_SOURCE_GENERATOR "ZIP")
  endif()

  # Mac OS X package
  if(CPACK_GENERATOR MATCHES "PackageMaker|DragNDrop")
    set(CPACK_PACKAGE_FILE_NAME
      "${CPACK_PACKAGE_FILE_NAME}-${CPACK_PACKAGE_VERSION}")
    set(CPACK_PACKAGING_INSTALL_PREFIX /usr/local)
  # Debian package
  elseif(CPACK_GENERATOR MATCHES "DEB")
    # https://github.com/hhvm/packaging/tree/master/hhvm/deb
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "binutils, libboost-filesystem1.54.0,"
      "libboost-program-options1.54.0, libboost-regex1.54.0,"
      "libboost-system1.54.0, libboost-thread1.54.0, libbz2-1.0,"
      "libc-client2007e, libc6, libcap2, libcomerr2, libcurl3 (>= 7.26.0),"
      "libedit2, libelf1, libevent-2.0-5, libexpat1, libfontconfig1, "
      "libfreetype6, libgcc1, libgcrypt11, libgd2-xpm-dev, libgnutls26, "
      "libgoogle-glog0, libgpg-error0, libgssapi-krb5-2, libicu52, libidn11, "
      "libjemalloc1 (>= 3.0.0), libjpeg8, libk5crypto3, libkeyutils1, "
      "libkrb5-3, libkrb5support0, libldap-2.4-2, libmagickwand5, libmcrypt4, "
      "libmemcached11, libmysqlclient18, libonig2, libp11-kit0, libpam0g, "
      "libpcre3, libpng12-0, libvpx-dev, libvpx1, libsasl2-2, libsqlite3-0, libssl1.0.0, "
      "libstdc++6, libtasn1-6, libtbb2, libtinfo5, libunwind8, libx11-6, "
      "libxau6, libxcb1, libxdmcp6, libxml2, libxpm4, libxslt1.1, zlib1g")
    set(CPACK_DEBIAN_PACKAGE_SECTION "web")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://hhvm.com")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}"
      "-${CPACK_PACKAGE_VERSION}"
      "-${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
  # RPM package
  elseif(CPACK_GENERATOR MATCHES "RPM")
    # https://github.com/pld-linux/hhvm
    # https://github.com/hhvm/packaging/tree/master/hhvm/rpm/fedora20/rpmbuild/
    set(CPACK_RPM_PACKAGE_REQUIRES "a52dec-libs-devel, apr-devel, autoconf, "
      "binutils-devel, boost-devel >= 1.50, cmake >= 2.8.7, "
      "curl-devel >= 7.29.0, elfutils-devel, expat-devel, "
      "gcc >= 6:4.6.0, gd-devel, glog-devel >= 0.3.2, ImageMagick-devel, "
      "imap-devel >= 1:2007, jemalloc-devel >= 3.0.0, libcap-devel, "
      "libdwarf-devel >= 20130729, libicu-devel >= 4.2, libmbfl-devel, "
      "libmcrypt-devel, libmemcached-devel >= 1.0.4, libxml2-devel, "
      "libstdc++-devel >= 6:4.3, libunwind-devel, libxslt-devel, "
      "mysql-devel, ocaml-findlib, oniguruma-devel, openssl-devel, "
      "pcre-devel, readline-devel, rpmbuild(macros) >= 1.675, "
      "tbb-devel >= 4.0.6000, zlib-devel")
    set(CPACK_RPM_PACKAGE_GROUP "Development/Languages")
    set(CPACK_RPM_PACKAGE_LICENSE "PHP 3.01 and BSD")
    set(CPACK_RPM_PACKAGE_URL "http://hhvm.com")
    set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}"
      "-${CPACK_PACKAGE_VERSION}"
      "-${CPACK_RPM_PACKAGE_ARCHITECTURE}")
  endif()
  include(CPack)
endif()
