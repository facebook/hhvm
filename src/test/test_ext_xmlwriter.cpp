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

#include <test/test_ext_xmlwriter.h>
#include <runtime/ext/ext_xmlwriter.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtXmlwriter::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_xmlwriter_open_memory);
  RUN_TEST(test_xmlwriter_open_uri);
  RUN_TEST(test_xmlwriter_set_indent_string);
  RUN_TEST(test_xmlwriter_set_indent);
  RUN_TEST(test_xmlwriter_start_document);
  RUN_TEST(test_xmlwriter_start_element);
  RUN_TEST(test_xmlwriter_start_element_ns);
  RUN_TEST(test_xmlwriter_write_element_ns);
  RUN_TEST(test_xmlwriter_write_element);
  RUN_TEST(test_xmlwriter_end_element);
  RUN_TEST(test_xmlwriter_full_end_element);
  RUN_TEST(test_xmlwriter_start_attribute_ns);
  RUN_TEST(test_xmlwriter_start_attribute);
  RUN_TEST(test_xmlwriter_write_attribute_ns);
  RUN_TEST(test_xmlwriter_write_attribute);
  RUN_TEST(test_xmlwriter_end_attribute);
  RUN_TEST(test_xmlwriter_start_cdata);
  RUN_TEST(test_xmlwriter_write_cdata);
  RUN_TEST(test_xmlwriter_end_cdata);
  RUN_TEST(test_xmlwriter_start_comment);
  RUN_TEST(test_xmlwriter_write_comment);
  RUN_TEST(test_xmlwriter_end_comment);
  RUN_TEST(test_xmlwriter_end_document);
  RUN_TEST(test_xmlwriter_start_pi);
  RUN_TEST(test_xmlwriter_write_pi);
  RUN_TEST(test_xmlwriter_end_pi);
  RUN_TEST(test_xmlwriter_text);
  RUN_TEST(test_xmlwriter_write_raw);
  RUN_TEST(test_xmlwriter_start_dtd);
  RUN_TEST(test_xmlwriter_write_dtd);
  RUN_TEST(test_xmlwriter_start_dtd_element);
  RUN_TEST(test_xmlwriter_write_dtd_element);
  RUN_TEST(test_xmlwriter_end_dtd_element);
  RUN_TEST(test_xmlwriter_start_dtd_attlist);
  RUN_TEST(test_xmlwriter_write_dtd_attlist);
  RUN_TEST(test_xmlwriter_end_dtd_attlist);
  RUN_TEST(test_xmlwriter_start_dtd_entity);
  RUN_TEST(test_xmlwriter_write_dtd_entity);
  RUN_TEST(test_xmlwriter_end_dtd_entity);
  RUN_TEST(test_xmlwriter_end_dtd);
  RUN_TEST(test_xmlwriter_flush);
  RUN_TEST(test_xmlwriter_output_memory);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtXmlwriter::test_xmlwriter_open_memory() {
  Variant xml = f_xmlwriter_open_memory();
  VERIFY(f_xmlwriter_set_indent(xml, true));
  VERIFY(f_xmlwriter_set_indent_string(xml, "  "));
  VERIFY(f_xmlwriter_start_document(xml, "1.0", "utf-8"));
  VERIFY(f_xmlwriter_start_element(xml, "node"));
  VERIFY(f_xmlwriter_write_attribute(xml, "name", "value"));
  VERIFY(f_xmlwriter_write_element(xml, "subnode", "some text"));
  VERIFY(f_xmlwriter_end_element(xml));
  VERIFY(f_xmlwriter_end_document(xml));
  String out = f_xmlwriter_flush(xml);
  VS(out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<node name=\"value\">\n  <subnode>some text</subnode>\n</node>\n");
  return Count(true);
}

bool TestExtXmlwriter::test_xmlwriter_open_uri() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_set_indent_string() {
  // tested in test_xmlwriter_open_memory
  return Count(true);
}

bool TestExtXmlwriter::test_xmlwriter_set_indent() {
  // tested in test_xmlwriter_open_memory
  return Count(true);
}

bool TestExtXmlwriter::test_xmlwriter_start_document() {
  // tested in test_xmlwriter_open_memory
  return Count(true);
}

bool TestExtXmlwriter::test_xmlwriter_start_element() {
  // tested in test_xmlwriter_open_memory
  return Count(true);
}

bool TestExtXmlwriter::test_xmlwriter_start_element_ns() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_write_element_ns() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_write_element() {
  // tested in test_xmlwriter_open_memory
  return Count(true);
}

bool TestExtXmlwriter::test_xmlwriter_end_element() {
  // tested in test_xmlwriter_open_memory
  return Count(true);
}

bool TestExtXmlwriter::test_xmlwriter_full_end_element() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_start_attribute_ns() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_start_attribute() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_write_attribute_ns() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_write_attribute() {
  // tested in test_xmlwriter_open_memory
  return Count(true);
}

bool TestExtXmlwriter::test_xmlwriter_end_attribute() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_start_cdata() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_write_cdata() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_end_cdata() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_start_comment() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_write_comment() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_end_comment() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_end_document() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_start_pi() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_write_pi() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_end_pi() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_text() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_write_raw() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_start_dtd() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_write_dtd() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_start_dtd_element() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_write_dtd_element() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_end_dtd_element() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_start_dtd_attlist() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_write_dtd_attlist() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_end_dtd_attlist() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_start_dtd_entity() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_write_dtd_entity() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_end_dtd_entity() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_end_dtd() {
  //VCB("<?php ");
  return true;
}

bool TestExtXmlwriter::test_xmlwriter_flush() {
  // tested in test_xmlwriter_open_memory
  return Count(true);
}

bool TestExtXmlwriter::test_xmlwriter_output_memory() {
  // tested in test_xmlwriter_open_memory
  return Count(true);
}
