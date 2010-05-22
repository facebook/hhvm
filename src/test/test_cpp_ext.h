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

#ifndef __TEST_CPP_EXT_H__
#define __TEST_CPP_EXT_H__

#include <test/test_cpp_base.h>
#include <runtime/ext/ext_variable.h> // we frequently need to call f_var_dump()

///////////////////////////////////////////////////////////////////////////////

/**
 * Testing CPP extension library coding.
 */
class TestCppExt : public TestCppBase {
public:
  TestCppExt();
};

///////////////////////////////////////////////////////////////////////////////

#endif // __TEST_CPP_EXT_H__
