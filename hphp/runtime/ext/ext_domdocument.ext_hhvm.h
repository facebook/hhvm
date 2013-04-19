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
namespace HPHP {

TypedValue* fh_dom_document_create_element(TypedValue* _rv, TypedValue* obj, Value* name, Value* value) asm("_ZN4HPHP29f_dom_document_create_elementERKNS_7VariantERKNS_6StringES5_");

TypedValue* fh_dom_document_create_document_fragment(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP39f_dom_document_create_document_fragmentERKNS_7VariantE");

TypedValue* fh_dom_document_create_text_node(TypedValue* _rv, TypedValue* obj, Value* data) asm("_ZN4HPHP31f_dom_document_create_text_nodeERKNS_7VariantERKNS_6StringE");

TypedValue* fh_dom_document_create_comment(TypedValue* _rv, TypedValue* obj, Value* data) asm("_ZN4HPHP29f_dom_document_create_commentERKNS_7VariantERKNS_6StringE");

TypedValue* fh_dom_document_create_cdatasection(TypedValue* _rv, TypedValue* obj, Value* data) asm("_ZN4HPHP34f_dom_document_create_cdatasectionERKNS_7VariantERKNS_6StringE");

TypedValue* fh_dom_document_create_processing_instruction(TypedValue* _rv, TypedValue* obj, Value* target, Value* data) asm("_ZN4HPHP44f_dom_document_create_processing_instructionERKNS_7VariantERKNS_6StringES5_");

TypedValue* fh_dom_document_create_attribute(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP31f_dom_document_create_attributeERKNS_7VariantERKNS_6StringE");

TypedValue* fh_dom_document_create_entity_reference(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP38f_dom_document_create_entity_referenceERKNS_7VariantERKNS_6StringE");

TypedValue* fh_dom_document_get_elements_by_tag_name(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP39f_dom_document_get_elements_by_tag_nameERKNS_7VariantERKNS_6StringE");

TypedValue* fh_dom_document_import_node(TypedValue* _rv, TypedValue* obj, Value* importednode, bool deep) asm("_ZN4HPHP26f_dom_document_import_nodeERKNS_7VariantERKNS_6ObjectEb");

TypedValue* fh_dom_document_create_element_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* qualifiedname, Value* value) asm("_ZN4HPHP32f_dom_document_create_element_nsERKNS_7VariantERKNS_6StringES5_S5_");

TypedValue* fh_dom_document_create_attribute_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* qualifiedname) asm("_ZN4HPHP34f_dom_document_create_attribute_nsERKNS_7VariantERKNS_6StringES5_");

TypedValue* fh_dom_document_get_elements_by_tag_name_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* localname) asm("_ZN4HPHP42f_dom_document_get_elements_by_tag_name_nsERKNS_7VariantERKNS_6StringES5_");

TypedValue* fh_dom_document_get_element_by_id(TypedValue* _rv, TypedValue* obj, Value* elementid) asm("_ZN4HPHP32f_dom_document_get_element_by_idERKNS_7VariantERKNS_6StringE");

TypedValue* fh_dom_document_normalize_document(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP33f_dom_document_normalize_documentERKNS_7VariantE");

TypedValue* fh_dom_document_save(TypedValue* _rv, TypedValue* obj, Value* file, long options) asm("_ZN4HPHP19f_dom_document_saveERKNS_7VariantERKNS_6StringEl");

TypedValue* fh_dom_document_savexml(TypedValue* _rv, TypedValue* obj, Value* node, long options) asm("_ZN4HPHP22f_dom_document_savexmlERKNS_7VariantERKNS_6ObjectEl");

TypedValue* fh_dom_document_validate(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP23f_dom_document_validateERKNS_7VariantE");

TypedValue* fh_dom_document_xinclude(TypedValue* _rv, TypedValue* obj, long options) asm("_ZN4HPHP23f_dom_document_xincludeERKNS_7VariantEl");

TypedValue* fh_dom_document_save_html(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP24f_dom_document_save_htmlERKNS_7VariantE");

TypedValue* fh_dom_document_save_html_file(TypedValue* _rv, TypedValue* obj, Value* file) asm("_ZN4HPHP29f_dom_document_save_html_fileERKNS_7VariantERKNS_6StringE");

TypedValue* fh_dom_document_schema_validate_file(TypedValue* _rv, TypedValue* obj, Value* filename) asm("_ZN4HPHP35f_dom_document_schema_validate_fileERKNS_7VariantERKNS_6StringE");

TypedValue* fh_dom_document_schema_validate_xml(TypedValue* _rv, TypedValue* obj, Value* source) asm("_ZN4HPHP34f_dom_document_schema_validate_xmlERKNS_7VariantERKNS_6StringE");

TypedValue* fh_dom_document_relaxng_validate_file(TypedValue* _rv, TypedValue* obj, Value* filename) asm("_ZN4HPHP36f_dom_document_relaxng_validate_fileERKNS_7VariantERKNS_6StringE");

TypedValue* fh_dom_document_relaxng_validate_xml(TypedValue* _rv, TypedValue* obj, Value* source) asm("_ZN4HPHP35f_dom_document_relaxng_validate_xmlERKNS_7VariantERKNS_6StringE");

TypedValue* fh_dom_node_insert_before(TypedValue* _rv, TypedValue* obj, Value* newnode, Value* refnode) asm("_ZN4HPHP24f_dom_node_insert_beforeERKNS_7VariantERKNS_6ObjectES5_");

TypedValue* fh_dom_node_replace_child(TypedValue* _rv, TypedValue* obj, Value* newchildobj, Value* oldchildobj) asm("_ZN4HPHP24f_dom_node_replace_childERKNS_7VariantERKNS_6ObjectES5_");

TypedValue* fh_dom_node_remove_child(TypedValue* _rv, TypedValue* obj, Value* node) asm("_ZN4HPHP23f_dom_node_remove_childERKNS_7VariantERKNS_6ObjectE");

TypedValue* fh_dom_node_append_child(TypedValue* _rv, TypedValue* obj, Value* newnode) asm("_ZN4HPHP23f_dom_node_append_childERKNS_7VariantERKNS_6ObjectE");

TypedValue* fh_dom_node_has_child_nodes(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP26f_dom_node_has_child_nodesERKNS_7VariantE");

TypedValue* fh_dom_node_clone_node(TypedValue* _rv, TypedValue* obj, bool deep) asm("_ZN4HPHP21f_dom_node_clone_nodeERKNS_7VariantEb");

TypedValue* fh_dom_node_normalize(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP20f_dom_node_normalizeERKNS_7VariantE");

TypedValue* fh_dom_node_is_supported(TypedValue* _rv, TypedValue* obj, Value* feature, Value* version) asm("_ZN4HPHP23f_dom_node_is_supportedERKNS_7VariantERKNS_6StringES5_");

TypedValue* fh_dom_node_has_attributes(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP25f_dom_node_has_attributesERKNS_7VariantE");

TypedValue* fh_dom_node_is_same_node(TypedValue* _rv, TypedValue* obj, Value* node) asm("_ZN4HPHP23f_dom_node_is_same_nodeERKNS_7VariantERKNS_6ObjectE");

TypedValue* fh_dom_node_lookup_prefix(TypedValue* _rv, TypedValue* obj, Value* prefix) asm("_ZN4HPHP24f_dom_node_lookup_prefixERKNS_7VariantERKNS_6StringE");

TypedValue* fh_dom_node_is_default_namespace(TypedValue* _rv, TypedValue* obj, Value* namespaceuri) asm("_ZN4HPHP31f_dom_node_is_default_namespaceERKNS_7VariantERKNS_6StringE");

TypedValue* fh_dom_node_lookup_namespace_uri(TypedValue* _rv, TypedValue* obj, Value* namespaceuri) asm("_ZN4HPHP31f_dom_node_lookup_namespace_uriERKNS_7VariantERKNS_6StringE");

TypedValue* fh_dom_nodelist_item(TypedValue* _rv, TypedValue* obj, long index) asm("_ZN4HPHP19f_dom_nodelist_itemERKNS_7VariantEl");

TypedValue* fh_dom_namednodemap_get_named_item(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP33f_dom_namednodemap_get_named_itemERKNS_7VariantERKNS_6StringE");

TypedValue* fh_dom_namednodemap_item(TypedValue* _rv, TypedValue* obj, long index) asm("_ZN4HPHP23f_dom_namednodemap_itemERKNS_7VariantEl");

TypedValue* fh_dom_namednodemap_get_named_item_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* localname) asm("_ZN4HPHP36f_dom_namednodemap_get_named_item_nsERKNS_7VariantERKNS_6StringES5_");

TypedValue* fh_dom_characterdata_substring_data(TypedValue* _rv, TypedValue* obj, long offset, long count) asm("_ZN4HPHP34f_dom_characterdata_substring_dataERKNS_7VariantEll");

TypedValue* fh_dom_characterdata_append_data(TypedValue* _rv, TypedValue* obj, Value* arg) asm("_ZN4HPHP31f_dom_characterdata_append_dataERKNS_7VariantERKNS_6StringE");

TypedValue* fh_dom_characterdata_insert_data(TypedValue* _rv, TypedValue* obj, long offset, Value* data) asm("_ZN4HPHP31f_dom_characterdata_insert_dataERKNS_7VariantElRKNS_6StringE");

TypedValue* fh_dom_characterdata_delete_data(TypedValue* _rv, TypedValue* obj, long offset, long count) asm("_ZN4HPHP31f_dom_characterdata_delete_dataERKNS_7VariantEll");

TypedValue* fh_dom_characterdata_replace_data(TypedValue* _rv, TypedValue* obj, long offset, long count, Value* data) asm("_ZN4HPHP32f_dom_characterdata_replace_dataERKNS_7VariantEllRKNS_6StringE");

TypedValue* fh_dom_attr_is_id(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP16f_dom_attr_is_idERKNS_7VariantE");

TypedValue* fh_dom_element_get_attribute(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP27f_dom_element_get_attributeERKNS_7VariantERKNS_6StringE");

TypedValue* fh_dom_element_set_attribute(TypedValue* _rv, TypedValue* obj, Value* name, Value* value) asm("_ZN4HPHP27f_dom_element_set_attributeERKNS_7VariantERKNS_6StringES5_");

TypedValue* fh_dom_element_remove_attribute(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP30f_dom_element_remove_attributeERKNS_7VariantERKNS_6StringE");

TypedValue* fh_dom_element_get_attribute_node(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP32f_dom_element_get_attribute_nodeERKNS_7VariantERKNS_6StringE");

TypedValue* fh_dom_element_set_attribute_node(TypedValue* _rv, TypedValue* obj, Value* newattr) asm("_ZN4HPHP32f_dom_element_set_attribute_nodeERKNS_7VariantERKNS_6ObjectE");

TypedValue* fh_dom_element_remove_attribute_node(TypedValue* _rv, TypedValue* obj, Value* oldattr) asm("_ZN4HPHP35f_dom_element_remove_attribute_nodeERKNS_7VariantERKNS_6ObjectE");

TypedValue* fh_dom_element_get_elements_by_tag_name(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP38f_dom_element_get_elements_by_tag_nameERKNS_7VariantERKNS_6StringE");

TypedValue* fh_dom_element_get_attribute_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* localname) asm("_ZN4HPHP30f_dom_element_get_attribute_nsERKNS_7VariantERKNS_6StringES5_");

TypedValue* fh_dom_element_set_attribute_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* name, Value* value) asm("_ZN4HPHP30f_dom_element_set_attribute_nsERKNS_7VariantERKNS_6StringES5_S5_");

TypedValue* fh_dom_element_remove_attribute_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* localname) asm("_ZN4HPHP33f_dom_element_remove_attribute_nsERKNS_7VariantERKNS_6StringES5_");

TypedValue* fh_dom_element_get_attribute_node_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* localname) asm("_ZN4HPHP35f_dom_element_get_attribute_node_nsERKNS_7VariantERKNS_6StringES5_");

TypedValue* fh_dom_element_set_attribute_node_ns(TypedValue* _rv, TypedValue* obj, Value* newattr) asm("_ZN4HPHP35f_dom_element_set_attribute_node_nsERKNS_7VariantERKNS_6ObjectE");

TypedValue* fh_dom_element_get_elements_by_tag_name_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* localname) asm("_ZN4HPHP41f_dom_element_get_elements_by_tag_name_nsERKNS_7VariantERKNS_6StringES5_");

TypedValue* fh_dom_element_has_attribute(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP27f_dom_element_has_attributeERKNS_7VariantERKNS_6StringE");

TypedValue* fh_dom_element_has_attribute_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* localname) asm("_ZN4HPHP30f_dom_element_has_attribute_nsERKNS_7VariantERKNS_6StringES5_");

TypedValue* fh_dom_element_set_id_attribute(TypedValue* _rv, TypedValue* obj, Value* name, bool isid) asm("_ZN4HPHP30f_dom_element_set_id_attributeERKNS_7VariantERKNS_6StringEb");

TypedValue* fh_dom_element_set_id_attribute_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* localname, bool isid) asm("_ZN4HPHP33f_dom_element_set_id_attribute_nsERKNS_7VariantERKNS_6StringES5_b");

TypedValue* fh_dom_element_set_id_attribute_node(TypedValue* _rv, TypedValue* obj, Value* idattr, bool isid) asm("_ZN4HPHP35f_dom_element_set_id_attribute_nodeERKNS_7VariantERKNS_6ObjectEb");

TypedValue* fh_dom_text_split_text(TypedValue* _rv, TypedValue* obj, long offset) asm("_ZN4HPHP21f_dom_text_split_textERKNS_7VariantEl");

TypedValue* fh_dom_text_is_whitespace_in_element_content(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP43f_dom_text_is_whitespace_in_element_contentERKNS_7VariantE");

TypedValue* fh_dom_xpath_register_ns(TypedValue* _rv, TypedValue* obj, Value* prefix, Value* uri) asm("_ZN4HPHP23f_dom_xpath_register_nsERKNS_7VariantERKNS_6StringES5_");

TypedValue* fh_dom_xpath_query(TypedValue* _rv, TypedValue* obj, Value* expr, Value* context) asm("_ZN4HPHP17f_dom_xpath_queryERKNS_7VariantERKNS_6StringERKNS_6ObjectE");

TypedValue* fh_dom_xpath_evaluate(TypedValue* _rv, TypedValue* obj, Value* expr, Value* context) asm("_ZN4HPHP20f_dom_xpath_evaluateERKNS_7VariantERKNS_6StringERKNS_6ObjectE");

TypedValue* fh_dom_xpath_register_php_functions(TypedValue* _rv, TypedValue* obj, TypedValue* funcs) asm("_ZN4HPHP34f_dom_xpath_register_php_functionsERKNS_7VariantES2_");

} // namespace HPHP
