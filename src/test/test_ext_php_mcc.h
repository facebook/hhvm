/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __TEST_EXT_PHP_MCC_H__
#define __TEST_EXT_PHP_MCC_H__

#include <test/test_cpp_ext.h>
#include <cpp/ext/phpmcc/ext_php_mcc.h>

///////////////////////////////////////////////////////////////////////////////

class TestExtPhp_mcc : public TestCppExt {
public:
  TestExtPhp_mcc();

  virtual bool RunTests(const std::string &which);

  bool test_basic();

  void run_server();
  bool get_client(bool consistent_hashing, p_phpmcc &mcc);

private:
  int m_server_tcp_port;
  int m_server_udp_port;

  static bool IsPortInUse(int port);
  static int FindFreePort();
};

///////////////////////////////////////////////////////////////////////////////

#endif // __TEST_EXT_PHP_MCC_H__
