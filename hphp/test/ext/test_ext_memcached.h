/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_TEST_EXT_MEMCACHED_H_
#define incl_HPHP_TEST_EXT_MEMCACHED_H_

#include "hphp/test/ext/test_cpp_ext.h"

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

#endif // incl_HPHP_TEST_EXT_MEMCACHED_H_
