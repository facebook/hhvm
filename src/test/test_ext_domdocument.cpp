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

#include <test/test_ext_domdocument.h>
#include <cpp/ext/ext_domdocument.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtDomdocument::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_dom_node_append_child);
  RUN_TEST(test_dom_node_clone_node);
  RUN_TEST(test_dom_node_get_line_no);
  RUN_TEST(test_dom_node_has_attributes);
  RUN_TEST(test_dom_node_has_child_nodes);
  RUN_TEST(test_dom_node_insert_before);
  RUN_TEST(test_dom_node_is_default_namespace);
  RUN_TEST(test_dom_node_is_same_node);
  RUN_TEST(test_dom_node_is_supported);
  RUN_TEST(test_dom_node_lookup_namespace_uri);
  RUN_TEST(test_dom_node_lookup_prefix);
  RUN_TEST(test_dom_node_normalize);
  RUN_TEST(test_dom_node_remove_child);
  RUN_TEST(test_dom_node_replace_child);
  RUN_TEST(test_dom_node_c14n);
  RUN_TEST(test_dom_node_c14n_file);
  RUN_TEST(test_dom_node_get_node_path);
  RUN_TEST(test_dom_attr_is_id);
  RUN_TEST(test_dom_characterdata_append_data);
  RUN_TEST(test_dom_characterdata_delete_data);
  RUN_TEST(test_dom_characterdata_insert_data);
  RUN_TEST(test_dom_characterdata_replace_data);
  RUN_TEST(test_dom_characterdata_substring_data);
  RUN_TEST(test_dom_text_is_whitespace_in_element_content);
  RUN_TEST(test_dom_text_split_text);
  RUN_TEST(test_dom_document_create_attribute);
  RUN_TEST(test_dom_document_create_attribute_ns);
  RUN_TEST(test_dom_document_create_cdatasection);
  RUN_TEST(test_dom_document_create_comment);
  RUN_TEST(test_dom_document_create_document_fragment);
  RUN_TEST(test_dom_document_create_element);
  RUN_TEST(test_dom_document_create_element_ns);
  RUN_TEST(test_dom_document_create_entity_reference);
  RUN_TEST(test_dom_document_create_processing_instruction);
  RUN_TEST(test_dom_document_create_text_node);
  RUN_TEST(test_dom_document_get_element_by_id);
  RUN_TEST(test_dom_document_get_elements_by_tag_name);
  RUN_TEST(test_dom_document_get_elements_by_tag_name_ns);
  RUN_TEST(test_dom_document_import_node);
  RUN_TEST(test_dom_document_load);
  RUN_TEST(test_dom_document_load_html);
  RUN_TEST(test_dom_document_load_html_file);
  RUN_TEST(test_dom_document_load_xml);
  RUN_TEST(test_dom_document_normalize_document);
  RUN_TEST(test_dom_document_register_node_class);
  RUN_TEST(test_dom_document_relaxng_validate_file);
  RUN_TEST(test_dom_document_relaxng_validate_xml);
  RUN_TEST(test_dom_document_save);
  RUN_TEST(test_dom_document_save_html);
  RUN_TEST(test_dom_document_save_html_file);
  RUN_TEST(test_dom_document_savexml);
  RUN_TEST(test_dom_document_schema_validate_file);
  RUN_TEST(test_dom_document_schema_validate_xml);
  RUN_TEST(test_dom_document_validate);
  RUN_TEST(test_dom_document_xinclude);
  RUN_TEST(test_dom_document_fragment_append_xml);
  RUN_TEST(test_dom_element_get_attribute);
  RUN_TEST(test_dom_element_get_attribute_node);
  RUN_TEST(test_dom_element_get_attribute_node_ns);
  RUN_TEST(test_dom_element_get_attribute_ns);
  RUN_TEST(test_dom_element_get_elements_by_tag_name);
  RUN_TEST(test_dom_element_get_elements_by_tag_name_ns);
  RUN_TEST(test_dom_element_has_attribute);
  RUN_TEST(test_dom_element_has_attribute_ns);
  RUN_TEST(test_dom_element_remove_attribute);
  RUN_TEST(test_dom_element_remove_attribute_node);
  RUN_TEST(test_dom_element_remove_attribute_ns);
  RUN_TEST(test_dom_element_set_attribute);
  RUN_TEST(test_dom_element_set_attribute_node);
  RUN_TEST(test_dom_element_set_attribute_node_ns);
  RUN_TEST(test_dom_element_set_attribute_ns);
  RUN_TEST(test_dom_element_set_id_attribute);
  RUN_TEST(test_dom_element_set_id_attribute_node);
  RUN_TEST(test_dom_element_set_id_attribute_ns);
  RUN_TEST(test_dom_namednodemap_get_named_item);
  RUN_TEST(test_dom_namednodemap_get_named_item_ns);
  RUN_TEST(test_dom_namednodemap_item);
  RUN_TEST(test_dom_nodelist_item);
  RUN_TEST(test_dom_implementation_create_document);
  RUN_TEST(test_dom_implementation_create_document_type);
  RUN_TEST(test_dom_implementation_has_feature);
  RUN_TEST(test_dom_xpath_evaluate);
  RUN_TEST(test_dom_xpath_query);
  RUN_TEST(test_dom_xpath_register_ns);
  RUN_TEST(test_dom_xpath_register_php_functions);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtDomdocument::test_dom_node_append_child() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_node_clone_node() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_node_get_line_no() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_node_has_attributes() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_node_has_child_nodes() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_node_insert_before() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_node_is_default_namespace() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_node_is_same_node() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_node_is_supported() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_node_lookup_namespace_uri() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_node_lookup_prefix() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_node_normalize() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_node_remove_child() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_node_replace_child() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_node_c14n() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_node_c14n_file() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_node_get_node_path() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_attr_is_id() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_characterdata_append_data() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_characterdata_delete_data() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_characterdata_insert_data() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_characterdata_replace_data() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_characterdata_substring_data() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_text_is_whitespace_in_element_content() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_text_split_text() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_create_attribute() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_create_attribute_ns() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_create_cdatasection() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_create_comment() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_create_document_fragment() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_create_element() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_create_element_ns() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_create_entity_reference() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_create_processing_instruction() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_create_text_node() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_get_element_by_id() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_get_elements_by_tag_name() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_get_elements_by_tag_name_ns() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_import_node() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_load() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_load_html() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_load_html_file() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_load_xml() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_normalize_document() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_register_node_class() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_relaxng_validate_file() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_relaxng_validate_xml() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_save() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_save_html() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_save_html_file() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_savexml() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_schema_validate_file() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_schema_validate_xml() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_validate() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_xinclude() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_document_fragment_append_xml() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_element_get_attribute() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_element_get_attribute_node() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_element_get_attribute_node_ns() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_element_get_attribute_ns() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_element_get_elements_by_tag_name() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_element_get_elements_by_tag_name_ns() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_element_has_attribute() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_element_has_attribute_ns() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_element_remove_attribute() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_element_remove_attribute_node() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_element_remove_attribute_ns() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_element_set_attribute() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_element_set_attribute_node() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_element_set_attribute_node_ns() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_element_set_attribute_ns() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_element_set_id_attribute() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_element_set_id_attribute_node() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_element_set_id_attribute_ns() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_namednodemap_get_named_item() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_namednodemap_get_named_item_ns() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_namednodemap_item() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_nodelist_item() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_implementation_create_document() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_implementation_create_document_type() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_implementation_has_feature() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_xpath_evaluate() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_xpath_query() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_xpath_register_ns() {
  return Count(true);
}

bool TestExtDomdocument::test_dom_xpath_register_php_functions() {
  return Count(true);
}
