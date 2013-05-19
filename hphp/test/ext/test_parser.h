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

#ifndef incl_HPHP_TEST_PARSER_H_
#define incl_HPHP_TEST_PARSER_H_

#include "hphp/test/ext/test_base.h"

///////////////////////////////////////////////////////////////////////////////

class TestParser : public TestBase {
 protected:
  bool VerifyParser(const char *input, const char *output,
                    const char *file = "", int line = 0,
                    const char *output2 = nullptr);
  bool SameCode(std::string code1, std::string code2);
};

///////////////////////////////////////////////////////////////////////////////
// macros

#define V(a,b)                                                          \
  if (!Count(VerifyParser(a,b,__FILE__,__LINE__))) return false;
#define V2(a,b,c)                                                       \
  if (!Count(VerifyParser(a,b,__FILE__,__LINE__,c))) return false;

///////////////////////////////////////////////////////////////////////////////

#endif // incl_HPHP_TEST_PARSER_H_
