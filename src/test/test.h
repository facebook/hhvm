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

#ifndef __TEST_H__
#define __TEST_H__

#include <lib/hphp.h>

///////////////////////////////////////////////////////////////////////////////

class Test {
 public:
  static int s_total;
  static int s_passed;
  static bool s_quiet; // less printf, good for nightly unit test runs

 public:
  Test() {}

  void RunTests(std::string &suite, std::string &which, std::string &set);
};

///////////////////////////////////////////////////////////////////////////////

#endif // __TEST_H__
