/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Hyves (http://www.hyves.nl)                       |
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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
   | Authors: Andrei Zmievski <andrei@php.net>                            |
   |          Lorenzo Castelli <lorenzo@hyves.nl> (Hiphop port)           |
   +----------------------------------------------------------------------+
*/

#ifndef __TEST_EXT_MEMCACHED_H__
#define __TEST_EXT_MEMCACHED_H__

#include <test/test_cpp_ext.h>

///////////////////////////////////////////////////////////////////////////////

class TestExtMemcached : public TestCppExt {
 public:
  virtual bool RunTests(const std::string &which);

  bool test_Memcached_construct_persistent();
  bool test_Memcached_get_set();
  bool test_Memcached_types();
  bool test_Memcached_cas();
  bool test_Memcached_delete();
};

///////////////////////////////////////////////////////////////////////////////

#endif // __TEST_EXT_MEMCACHED_H__
