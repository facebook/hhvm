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

#include <test/test_ext_xml.h>
#include <runtime/ext/ext_xml.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtXml::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_xml_parser_create);
  RUN_TEST(test_xml_parser_free);
  RUN_TEST(test_xml_parse);
  RUN_TEST(test_xml_parse_into_struct);
  RUN_TEST(test_xml_parser_create_ns);
  RUN_TEST(test_xml_parser_get_option);
  RUN_TEST(test_xml_parser_set_option);
  RUN_TEST(test_xml_set_character_data_handler);
  RUN_TEST(test_xml_set_default_handler);
  RUN_TEST(test_xml_set_element_handler);
  RUN_TEST(test_xml_set_processing_instruction_handler);
  RUN_TEST(test_xml_set_start_namespace_decl_handler);
  RUN_TEST(test_xml_set_end_namespace_decl_handler);
  RUN_TEST(test_xml_set_unparsed_entity_decl_handler);
  RUN_TEST(test_xml_set_external_entity_ref_handler);
  RUN_TEST(test_xml_set_notation_decl_handler);
  RUN_TEST(test_xml_set_object);
  RUN_TEST(test_xml_get_current_byte_index);
  RUN_TEST(test_xml_get_current_column_number);
  RUN_TEST(test_xml_get_current_line_number);
  RUN_TEST(test_xml_get_error_code);
  RUN_TEST(test_xml_error_string);
  RUN_TEST(test_utf8_decode);
  RUN_TEST(test_utf8_encode);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtXml::test_xml_parser_create() {
  //VCB("<?php ");
  return true;
}

bool TestExtXml::test_xml_parser_free() {
  //VCB("<?php ");
  return true;
}

bool TestExtXml::test_xml_parse() {
  //VCB("<?php ");
  return true;
}

bool TestExtXml::test_xml_parse_into_struct() {
  //VCB("<?php ");
  String simple = "<para><note attrib1='foo'>simple&amp;note</note></para>";
  Variant p = f_xml_parser_create();
  Variant vals, index;
  f_xml_parse_into_struct(p, simple, ref(vals), ref(index));
  f_xml_parser_free(p);
  VS(f_print_r(index.rvalAt("PARA"),1),
    "Array\n(\n    [0] => 0\n    [1] => 2\n)\n");
  VS(f_print_r(index.rvalAt("NOTE"),1),
    "Array\n(\n    [0] => 1\n)\n");
  VS(f_print_r(vals.rvalAt(0),1),
    "Array\n(\n    [tag] => PARA\n    [type] => open\n    [level] => 1\n)\n");
  VS(f_print_r(vals.rvalAt(1),1),
    "Array\n(\n    [tag] => NOTE\n    [type] => complete\n    [level] => 2\n"
    "    [attributes] => Array\n        (\n            [ATTRIB1] => foo\n"
    "        )\n\n    [value] => simple&note\n)\n");
  VS(f_print_r(vals.rvalAt(2),1),
    "Array\n(\n    [tag] => PARA\n    [type] => close\n    [level] => 1\n)\n");
  return true;
}

bool TestExtXml::test_xml_parser_create_ns() {
  //VCB("<?php ");
  return true;
}

bool TestExtXml::test_xml_parser_get_option() {
  //VCB("<?php ");
  return true;
}

bool TestExtXml::test_xml_parser_set_option() {
  //VCB("<?php ");
  return true;
}

bool TestExtXml::test_xml_set_character_data_handler() {
  //VCB("<?php ");
  return true;
}

bool TestExtXml::test_xml_set_default_handler() {
  //VCB("<?php ");
  return true;
}

bool TestExtXml::test_xml_set_element_handler() {
  //VCB("<?php ");
  return true;
}

bool TestExtXml::test_xml_set_processing_instruction_handler() {
  //VCB("<?php ");
  return true;
}

bool TestExtXml::test_xml_set_start_namespace_decl_handler() {
  //VCB("<?php ");
  return true;
}

bool TestExtXml::test_xml_set_end_namespace_decl_handler() {
  //VCB("<?php ");
  return true;
}

bool TestExtXml::test_xml_set_unparsed_entity_decl_handler() {
  //VCB("<?php ");
  return true;
}

bool TestExtXml::test_xml_set_external_entity_ref_handler() {
  //VCB("<?php ");
  return true;
}

bool TestExtXml::test_xml_set_notation_decl_handler() {
  //VCB("<?php ");
  return true;
}

bool TestExtXml::test_xml_set_object() {
  //VCB("<?php ");
  return true;
}

bool TestExtXml::test_xml_get_current_byte_index() {
  //VCB("<?php ");
  return true;
}

bool TestExtXml::test_xml_get_current_column_number() {
  //VCB("<?php ");
  return true;
}

bool TestExtXml::test_xml_get_current_line_number() {
  //VCB("<?php ");
  return true;
}

bool TestExtXml::test_xml_get_error_code() {
  //VCB("<?php ");
  return true;
}

bool TestExtXml::test_xml_error_string() {
  //VCB("<?php ");
  return true;
}

bool TestExtXml::test_utf8_decode() {
  VS(f_utf8_decode("abc \xc3\x80 def"), "abc \xc0 def");
  return Count(true);
}

bool TestExtXml::test_utf8_encode() {
  VS(f_utf8_encode("abc \xc0 def"), "abc \xc3\x80 def");
  return Count(true);
}
