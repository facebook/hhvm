/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "test_ext_iplocation.h"
#include "ext_iplocation.h"
#include <util/logger.h>

IMPLEMENT_SEP_EXTENSION_TEST(Iplocation);
///////////////////////////////////////////////////////////////////////////////

bool TestExtIplocation::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_ip_get_location);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtIplocation::test_ip_get_location() {
  //IpLocMap::init(String("/home/admin/php/lib/php/extensions/no-debug-zts-20090626/ipdata.txt"));
  String ip = String("202.98.0.68");
  String addr =  f_ip_get_location(ip);
  printf("%s\n",addr.data());
  return Count(true);
}
