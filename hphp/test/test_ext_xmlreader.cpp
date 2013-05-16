/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "hphp/test/test_ext_xmlreader.h"
#include "hphp/runtime/ext/ext_xmlreader.h"

IMPLEMENT_SEP_EXTENSION_TEST(Xmlreader);
///////////////////////////////////////////////////////////////////////////////

bool TestExtXmlreader::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_XMLReader);
  RUN_TEST(test_XMLReader_getattribute);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtXmlreader::test_XMLReader() {
  return Count(true);
}

bool TestExtXmlreader::test_XMLReader_getattribute() {
  p_XMLReader reader(NEWOBJ(c_XMLReader)());
  reader->t_xml("<?xml version=\"1.0\" encoding=\"UTF-8\"?><a y=\"\" z=\"1\"></a>");
  reader->t_read();
  VS(reader->t_getattribute("x"), uninit_null());
  VS(reader->t_getattribute("y"), String(""));
  VS(reader->t_getattribute("z"), String("1"));
  return Count(true);
}
