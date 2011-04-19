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

#include <test/test_ext_simplexml.h>
#include <runtime/ext/ext_simplexml.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtSimplexml::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_simplexml_load_string);
  RUN_TEST(test_simplexml_load_file);
  RUN_TEST(test_libxml_get_errors);
  RUN_TEST(test_libxml_get_last_error);
  RUN_TEST(test_libxml_clear_errors);
  RUN_TEST(test_libxml_use_internal_errors);
  RUN_TEST(test_libxml_set_streams_context);
  RUN_TEST(test_libxml_disable_entity_loader);
  RUN_TEST(test_SimpleXMLElement);
  RUN_TEST(test_LibXMLError);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtSimplexml::test_simplexml_load_string() {
  // tested in TestCodeRun::TestSimpleXML
  return Count(true);
}

bool TestExtSimplexml::test_simplexml_load_file() {
  // nothing much different from simplexml_load_string()
  return Count(true);
}

bool TestExtSimplexml::test_libxml_get_errors() {
  VS(f_libxml_get_errors(), Array::Create());
  return Count(true);
}

bool TestExtSimplexml::test_libxml_get_last_error() {
  VS(f_libxml_get_last_error(), false);
  return Count(true);
}

bool TestExtSimplexml::test_libxml_clear_errors() {
  f_libxml_clear_errors();
  return Count(true);
}

bool TestExtSimplexml::test_libxml_use_internal_errors() {
  VS(f_libxml_use_internal_errors(), false);
  return Count(true);
}

bool TestExtSimplexml::test_libxml_set_streams_context() {
  try {
    f_libxml_set_streams_context(Object());
  } catch (NotImplementedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtSimplexml::test_libxml_disable_entity_loader() {
  try {
    f_libxml_disable_entity_loader(true);
  } catch (NotImplementedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtSimplexml::test_SimpleXMLElement() {
  // tested in simplexml functions
  return Count(true);
}

bool TestExtSimplexml::test_LibXMLError() {
  // tested in libxml functions
  return Count(true);
}
