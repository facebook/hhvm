#
#   +----------------------------------------------------------------------+
#   | HipHop for PHP                                                       |
#   +----------------------------------------------------------------------+
#   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
#   | Copyright (c) 1997-2010 The PHP Group                                |
#   +----------------------------------------------------------------------+
#   | This source file is subject to version 3.01 of the PHP license,      |
#   | that is bundled with this package in the file LICENSE, and is        |
#   | available through the world-wide-web at the following url:           |
#   | http://www.php.net/license/3_01.txt                                  |
#   | If you did not receive a copy of the PHP license and are unable to   |
#   | obtain it through the world-wide-web, please send a note to          |
#   | license@php.net so we can mail you a copy immediately.               |
#   +----------------------------------------------------------------------+
#

include(HPHPSetup)
include(FollySetup)
include(ExtZendCompat)

add_definitions("-DHHVM")

add_subdirectory(tools/bootstrap)

add_subdirectory(third_party/libafdt)
add_subdirectory(third_party/libmbfl)
add_subdirectory(third_party/libsqlite3)
add_subdirectory(third_party/timelib)
add_subdirectory(third_party/lz4)
add_subdirectory(third_party/double-conversion)
add_subdirectory(third_party/folly)
add_subdirectory(third_party/libzip)
if(ENABLE_FASTCGI)
	add_subdirectory(third_party/ti)
	add_subdirectory(third_party/thrift)
endif()

add_subdirectory(compiler)
add_subdirectory(hack)
add_subdirectory(hhbbc)
add_subdirectory(neo)
add_subdirectory(parser)

add_subdirectory(runtime)
add_subdirectory(runtime/ext)
add_subdirectory(runtime/ext_hhvm)
add_subdirectory(system)
add_subdirectory(util)
add_subdirectory(vixl)
add_subdirectory(zend)

add_subdirectory(hhvm)

if (NOT "$ENV{HPHP_NOTEST}" STREQUAL "1")
	add_subdirectory(test)
endif ()

# Keep this last
add_subdirectory(tools/hphpize)
