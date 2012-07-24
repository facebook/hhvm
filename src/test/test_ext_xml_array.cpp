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

#include "test_ext_xml_array.h"
#include <runtime/ext/ext_xml_array.h>
#include <runtime/ext/ext_variable.h>
#include <runtime/ext/ext_file.h>

IMPLEMENT_SEP_EXTENSION_TEST(Xml_array);
///////////////////////////////////////////////////////////////////////////////

bool TestExtXml_array::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_xml_array);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtXml_array::test_xml_array() {
  String s = f_file_get_contents("http://news.baidu.com/n?cmd=1&class=civilnews&tn=rss");
  f_var_dump(s);
  Array r = f_xml_array(s);
  f_var_dump(r);
  return Count(true);
}
