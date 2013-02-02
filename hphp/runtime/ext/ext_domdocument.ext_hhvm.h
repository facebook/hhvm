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

/*
HPHP::Variant HPHP::f_dom_document_create_element(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP29f_dom_document_create_elementERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
name => rdx
value => rcx
*/

TypedValue* fh_dom_document_create_element(TypedValue* _rv, TypedValue* obj, Value* name, Value* value) asm("_ZN4HPHP29f_dom_document_create_elementERKNS_7VariantERKNS_6StringES5_");

/*
HPHP::Variant HPHP::f_dom_document_create_document_fragment(HPHP::Variant const&)
_ZN4HPHP39f_dom_document_create_document_fragmentERKNS_7VariantE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_dom_document_create_document_fragment(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP39f_dom_document_create_document_fragmentERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_dom_document_create_text_node(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP31f_dom_document_create_text_nodeERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
data => rdx
*/

TypedValue* fh_dom_document_create_text_node(TypedValue* _rv, TypedValue* obj, Value* data) asm("_ZN4HPHP31f_dom_document_create_text_nodeERKNS_7VariantERKNS_6StringE");

/*
HPHP::Variant HPHP::f_dom_document_create_comment(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP29f_dom_document_create_commentERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
data => rdx
*/

TypedValue* fh_dom_document_create_comment(TypedValue* _rv, TypedValue* obj, Value* data) asm("_ZN4HPHP29f_dom_document_create_commentERKNS_7VariantERKNS_6StringE");

/*
HPHP::Variant HPHP::f_dom_document_create_cdatasection(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP34f_dom_document_create_cdatasectionERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
data => rdx
*/

TypedValue* fh_dom_document_create_cdatasection(TypedValue* _rv, TypedValue* obj, Value* data) asm("_ZN4HPHP34f_dom_document_create_cdatasectionERKNS_7VariantERKNS_6StringE");

/*
HPHP::Variant HPHP::f_dom_document_create_processing_instruction(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP44f_dom_document_create_processing_instructionERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
target => rdx
data => rcx
*/

TypedValue* fh_dom_document_create_processing_instruction(TypedValue* _rv, TypedValue* obj, Value* target, Value* data) asm("_ZN4HPHP44f_dom_document_create_processing_instructionERKNS_7VariantERKNS_6StringES5_");

/*
HPHP::Variant HPHP::f_dom_document_create_attribute(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP31f_dom_document_create_attributeERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
name => rdx
*/

TypedValue* fh_dom_document_create_attribute(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP31f_dom_document_create_attributeERKNS_7VariantERKNS_6StringE");

/*
HPHP::Variant HPHP::f_dom_document_create_entity_reference(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP38f_dom_document_create_entity_referenceERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
name => rdx
*/

TypedValue* fh_dom_document_create_entity_reference(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP38f_dom_document_create_entity_referenceERKNS_7VariantERKNS_6StringE");

/*
HPHP::Variant HPHP::f_dom_document_get_elements_by_tag_name(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP39f_dom_document_get_elements_by_tag_nameERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
name => rdx
*/

TypedValue* fh_dom_document_get_elements_by_tag_name(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP39f_dom_document_get_elements_by_tag_nameERKNS_7VariantERKNS_6StringE");

/*
HPHP::Variant HPHP::f_dom_document_import_node(HPHP::Variant const&, HPHP::Object const&, bool)
_ZN4HPHP26f_dom_document_import_nodeERKNS_7VariantERKNS_6ObjectEb

(return value) => rax
_rv => rdi
obj => rsi
importednode => rdx
deep => rcx
*/

TypedValue* fh_dom_document_import_node(TypedValue* _rv, TypedValue* obj, Value* importednode, bool deep) asm("_ZN4HPHP26f_dom_document_import_nodeERKNS_7VariantERKNS_6ObjectEb");

/*
HPHP::Variant HPHP::f_dom_document_create_element_ns(HPHP::Variant const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP32f_dom_document_create_element_nsERKNS_7VariantERKNS_6StringES5_S5_

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
qualifiedname => rcx
value => r8
*/

TypedValue* fh_dom_document_create_element_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* qualifiedname, Value* value) asm("_ZN4HPHP32f_dom_document_create_element_nsERKNS_7VariantERKNS_6StringES5_S5_");

/*
HPHP::Variant HPHP::f_dom_document_create_attribute_ns(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP34f_dom_document_create_attribute_nsERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
qualifiedname => rcx
*/

TypedValue* fh_dom_document_create_attribute_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* qualifiedname) asm("_ZN4HPHP34f_dom_document_create_attribute_nsERKNS_7VariantERKNS_6StringES5_");

/*
HPHP::Variant HPHP::f_dom_document_get_elements_by_tag_name_ns(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP42f_dom_document_get_elements_by_tag_name_nsERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
localname => rcx
*/

TypedValue* fh_dom_document_get_elements_by_tag_name_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* localname) asm("_ZN4HPHP42f_dom_document_get_elements_by_tag_name_nsERKNS_7VariantERKNS_6StringES5_");

/*
HPHP::Variant HPHP::f_dom_document_get_element_by_id(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP32f_dom_document_get_element_by_idERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
elementid => rdx
*/

TypedValue* fh_dom_document_get_element_by_id(TypedValue* _rv, TypedValue* obj, Value* elementid) asm("_ZN4HPHP32f_dom_document_get_element_by_idERKNS_7VariantERKNS_6StringE");

/*
HPHP::Variant HPHP::f_dom_document_normalize_document(HPHP::Variant const&)
_ZN4HPHP33f_dom_document_normalize_documentERKNS_7VariantE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_dom_document_normalize_document(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP33f_dom_document_normalize_documentERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_dom_document_save(HPHP::Variant const&, HPHP::String const&, long long)
_ZN4HPHP19f_dom_document_saveERKNS_7VariantERKNS_6StringEx

(return value) => rax
_rv => rdi
obj => rsi
file => rdx
options => rcx
*/

TypedValue* fh_dom_document_save(TypedValue* _rv, TypedValue* obj, Value* file, long long options) asm("_ZN4HPHP19f_dom_document_saveERKNS_7VariantERKNS_6StringEx");

/*
HPHP::Variant HPHP::f_dom_document_savexml(HPHP::Variant const&, HPHP::Object const&, long long)
_ZN4HPHP22f_dom_document_savexmlERKNS_7VariantERKNS_6ObjectEx

(return value) => rax
_rv => rdi
obj => rsi
node => rdx
options => rcx
*/

TypedValue* fh_dom_document_savexml(TypedValue* _rv, TypedValue* obj, Value* node, long long options) asm("_ZN4HPHP22f_dom_document_savexmlERKNS_7VariantERKNS_6ObjectEx");

/*
HPHP::Variant HPHP::f_dom_document_validate(HPHP::Variant const&)
_ZN4HPHP23f_dom_document_validateERKNS_7VariantE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_dom_document_validate(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP23f_dom_document_validateERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_dom_document_xinclude(HPHP::Variant const&, long long)
_ZN4HPHP23f_dom_document_xincludeERKNS_7VariantEx

(return value) => rax
_rv => rdi
obj => rsi
options => rdx
*/

TypedValue* fh_dom_document_xinclude(TypedValue* _rv, TypedValue* obj, long long options) asm("_ZN4HPHP23f_dom_document_xincludeERKNS_7VariantEx");

/*
HPHP::Variant HPHP::f_dom_document_save_html(HPHP::Variant const&)
_ZN4HPHP24f_dom_document_save_htmlERKNS_7VariantE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_dom_document_save_html(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP24f_dom_document_save_htmlERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_dom_document_save_html_file(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP29f_dom_document_save_html_fileERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
file => rdx
*/

TypedValue* fh_dom_document_save_html_file(TypedValue* _rv, TypedValue* obj, Value* file) asm("_ZN4HPHP29f_dom_document_save_html_fileERKNS_7VariantERKNS_6StringE");

/*
HPHP::Variant HPHP::f_dom_document_schema_validate_file(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP35f_dom_document_schema_validate_fileERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
filename => rdx
*/

TypedValue* fh_dom_document_schema_validate_file(TypedValue* _rv, TypedValue* obj, Value* filename) asm("_ZN4HPHP35f_dom_document_schema_validate_fileERKNS_7VariantERKNS_6StringE");

/*
HPHP::Variant HPHP::f_dom_document_schema_validate_xml(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP34f_dom_document_schema_validate_xmlERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
source => rdx
*/

TypedValue* fh_dom_document_schema_validate_xml(TypedValue* _rv, TypedValue* obj, Value* source) asm("_ZN4HPHP34f_dom_document_schema_validate_xmlERKNS_7VariantERKNS_6StringE");

/*
HPHP::Variant HPHP::f_dom_document_relaxng_validate_file(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP36f_dom_document_relaxng_validate_fileERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
filename => rdx
*/

TypedValue* fh_dom_document_relaxng_validate_file(TypedValue* _rv, TypedValue* obj, Value* filename) asm("_ZN4HPHP36f_dom_document_relaxng_validate_fileERKNS_7VariantERKNS_6StringE");

/*
HPHP::Variant HPHP::f_dom_document_relaxng_validate_xml(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP35f_dom_document_relaxng_validate_xmlERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
source => rdx
*/

TypedValue* fh_dom_document_relaxng_validate_xml(TypedValue* _rv, TypedValue* obj, Value* source) asm("_ZN4HPHP35f_dom_document_relaxng_validate_xmlERKNS_7VariantERKNS_6StringE");

/*
HPHP::Variant HPHP::f_dom_node_insert_before(HPHP::Variant const&, HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP24f_dom_node_insert_beforeERKNS_7VariantERKNS_6ObjectES5_

(return value) => rax
_rv => rdi
obj => rsi
newnode => rdx
refnode => rcx
*/

TypedValue* fh_dom_node_insert_before(TypedValue* _rv, TypedValue* obj, Value* newnode, Value* refnode) asm("_ZN4HPHP24f_dom_node_insert_beforeERKNS_7VariantERKNS_6ObjectES5_");

/*
HPHP::Variant HPHP::f_dom_node_replace_child(HPHP::Variant const&, HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP24f_dom_node_replace_childERKNS_7VariantERKNS_6ObjectES5_

(return value) => rax
_rv => rdi
obj => rsi
newchildobj => rdx
oldchildobj => rcx
*/

TypedValue* fh_dom_node_replace_child(TypedValue* _rv, TypedValue* obj, Value* newchildobj, Value* oldchildobj) asm("_ZN4HPHP24f_dom_node_replace_childERKNS_7VariantERKNS_6ObjectES5_");

/*
HPHP::Variant HPHP::f_dom_node_remove_child(HPHP::Variant const&, HPHP::Object const&)
_ZN4HPHP23f_dom_node_remove_childERKNS_7VariantERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
node => rdx
*/

TypedValue* fh_dom_node_remove_child(TypedValue* _rv, TypedValue* obj, Value* node) asm("_ZN4HPHP23f_dom_node_remove_childERKNS_7VariantERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_dom_node_append_child(HPHP::Variant const&, HPHP::Object const&)
_ZN4HPHP23f_dom_node_append_childERKNS_7VariantERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
newnode => rdx
*/

TypedValue* fh_dom_node_append_child(TypedValue* _rv, TypedValue* obj, Value* newnode) asm("_ZN4HPHP23f_dom_node_append_childERKNS_7VariantERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_dom_node_has_child_nodes(HPHP::Variant const&)
_ZN4HPHP26f_dom_node_has_child_nodesERKNS_7VariantE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_dom_node_has_child_nodes(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP26f_dom_node_has_child_nodesERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_dom_node_clone_node(HPHP::Variant const&, bool)
_ZN4HPHP21f_dom_node_clone_nodeERKNS_7VariantEb

(return value) => rax
_rv => rdi
obj => rsi
deep => rdx
*/

TypedValue* fh_dom_node_clone_node(TypedValue* _rv, TypedValue* obj, bool deep) asm("_ZN4HPHP21f_dom_node_clone_nodeERKNS_7VariantEb");

/*
HPHP::Variant HPHP::f_dom_node_normalize(HPHP::Variant const&)
_ZN4HPHP20f_dom_node_normalizeERKNS_7VariantE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_dom_node_normalize(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP20f_dom_node_normalizeERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_dom_node_is_supported(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP23f_dom_node_is_supportedERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
feature => rdx
version => rcx
*/

TypedValue* fh_dom_node_is_supported(TypedValue* _rv, TypedValue* obj, Value* feature, Value* version) asm("_ZN4HPHP23f_dom_node_is_supportedERKNS_7VariantERKNS_6StringES5_");

/*
HPHP::Variant HPHP::f_dom_node_has_attributes(HPHP::Variant const&)
_ZN4HPHP25f_dom_node_has_attributesERKNS_7VariantE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_dom_node_has_attributes(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP25f_dom_node_has_attributesERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_dom_node_is_same_node(HPHP::Variant const&, HPHP::Object const&)
_ZN4HPHP23f_dom_node_is_same_nodeERKNS_7VariantERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
node => rdx
*/

TypedValue* fh_dom_node_is_same_node(TypedValue* _rv, TypedValue* obj, Value* node) asm("_ZN4HPHP23f_dom_node_is_same_nodeERKNS_7VariantERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_dom_node_lookup_prefix(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP24f_dom_node_lookup_prefixERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
prefix => rdx
*/

TypedValue* fh_dom_node_lookup_prefix(TypedValue* _rv, TypedValue* obj, Value* prefix) asm("_ZN4HPHP24f_dom_node_lookup_prefixERKNS_7VariantERKNS_6StringE");

/*
HPHP::Variant HPHP::f_dom_node_is_default_namespace(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP31f_dom_node_is_default_namespaceERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
*/

TypedValue* fh_dom_node_is_default_namespace(TypedValue* _rv, TypedValue* obj, Value* namespaceuri) asm("_ZN4HPHP31f_dom_node_is_default_namespaceERKNS_7VariantERKNS_6StringE");

/*
HPHP::Variant HPHP::f_dom_node_lookup_namespace_uri(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP31f_dom_node_lookup_namespace_uriERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
*/

TypedValue* fh_dom_node_lookup_namespace_uri(TypedValue* _rv, TypedValue* obj, Value* namespaceuri) asm("_ZN4HPHP31f_dom_node_lookup_namespace_uriERKNS_7VariantERKNS_6StringE");

/*
HPHP::Variant HPHP::f_dom_nodelist_item(HPHP::Variant const&, long long)
_ZN4HPHP19f_dom_nodelist_itemERKNS_7VariantEx

(return value) => rax
_rv => rdi
obj => rsi
index => rdx
*/

TypedValue* fh_dom_nodelist_item(TypedValue* _rv, TypedValue* obj, long long index) asm("_ZN4HPHP19f_dom_nodelist_itemERKNS_7VariantEx");

/*
HPHP::Variant HPHP::f_dom_namednodemap_get_named_item(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP33f_dom_namednodemap_get_named_itemERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
name => rdx
*/

TypedValue* fh_dom_namednodemap_get_named_item(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP33f_dom_namednodemap_get_named_itemERKNS_7VariantERKNS_6StringE");

/*
HPHP::Variant HPHP::f_dom_namednodemap_item(HPHP::Variant const&, long long)
_ZN4HPHP23f_dom_namednodemap_itemERKNS_7VariantEx

(return value) => rax
_rv => rdi
obj => rsi
index => rdx
*/

TypedValue* fh_dom_namednodemap_item(TypedValue* _rv, TypedValue* obj, long long index) asm("_ZN4HPHP23f_dom_namednodemap_itemERKNS_7VariantEx");

/*
HPHP::Variant HPHP::f_dom_namednodemap_get_named_item_ns(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP36f_dom_namednodemap_get_named_item_nsERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
localname => rcx
*/

TypedValue* fh_dom_namednodemap_get_named_item_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* localname) asm("_ZN4HPHP36f_dom_namednodemap_get_named_item_nsERKNS_7VariantERKNS_6StringES5_");

/*
HPHP::Variant HPHP::f_dom_characterdata_substring_data(HPHP::Variant const&, long long, long long)
_ZN4HPHP34f_dom_characterdata_substring_dataERKNS_7VariantExx

(return value) => rax
_rv => rdi
obj => rsi
offset => rdx
count => rcx
*/

TypedValue* fh_dom_characterdata_substring_data(TypedValue* _rv, TypedValue* obj, long long offset, long long count) asm("_ZN4HPHP34f_dom_characterdata_substring_dataERKNS_7VariantExx");

/*
HPHP::Variant HPHP::f_dom_characterdata_append_data(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP31f_dom_characterdata_append_dataERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
arg => rdx
*/

TypedValue* fh_dom_characterdata_append_data(TypedValue* _rv, TypedValue* obj, Value* arg) asm("_ZN4HPHP31f_dom_characterdata_append_dataERKNS_7VariantERKNS_6StringE");

/*
HPHP::Variant HPHP::f_dom_characterdata_insert_data(HPHP::Variant const&, long long, HPHP::String const&)
_ZN4HPHP31f_dom_characterdata_insert_dataERKNS_7VariantExRKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
offset => rdx
data => rcx
*/

TypedValue* fh_dom_characterdata_insert_data(TypedValue* _rv, TypedValue* obj, long long offset, Value* data) asm("_ZN4HPHP31f_dom_characterdata_insert_dataERKNS_7VariantExRKNS_6StringE");

/*
HPHP::Variant HPHP::f_dom_characterdata_delete_data(HPHP::Variant const&, long long, long long)
_ZN4HPHP31f_dom_characterdata_delete_dataERKNS_7VariantExx

(return value) => rax
_rv => rdi
obj => rsi
offset => rdx
count => rcx
*/

TypedValue* fh_dom_characterdata_delete_data(TypedValue* _rv, TypedValue* obj, long long offset, long long count) asm("_ZN4HPHP31f_dom_characterdata_delete_dataERKNS_7VariantExx");

/*
HPHP::Variant HPHP::f_dom_characterdata_replace_data(HPHP::Variant const&, long long, long long, HPHP::String const&)
_ZN4HPHP32f_dom_characterdata_replace_dataERKNS_7VariantExxRKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
offset => rdx
count => rcx
data => r8
*/

TypedValue* fh_dom_characterdata_replace_data(TypedValue* _rv, TypedValue* obj, long long offset, long long count, Value* data) asm("_ZN4HPHP32f_dom_characterdata_replace_dataERKNS_7VariantExxRKNS_6StringE");

/*
HPHP::Variant HPHP::f_dom_attr_is_id(HPHP::Variant const&)
_ZN4HPHP16f_dom_attr_is_idERKNS_7VariantE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_dom_attr_is_id(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP16f_dom_attr_is_idERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_dom_element_get_attribute(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP27f_dom_element_get_attributeERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
name => rdx
*/

TypedValue* fh_dom_element_get_attribute(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP27f_dom_element_get_attributeERKNS_7VariantERKNS_6StringE");

/*
HPHP::Variant HPHP::f_dom_element_set_attribute(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP27f_dom_element_set_attributeERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
name => rdx
value => rcx
*/

TypedValue* fh_dom_element_set_attribute(TypedValue* _rv, TypedValue* obj, Value* name, Value* value) asm("_ZN4HPHP27f_dom_element_set_attributeERKNS_7VariantERKNS_6StringES5_");

/*
HPHP::Variant HPHP::f_dom_element_remove_attribute(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP30f_dom_element_remove_attributeERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
name => rdx
*/

TypedValue* fh_dom_element_remove_attribute(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP30f_dom_element_remove_attributeERKNS_7VariantERKNS_6StringE");

/*
HPHP::Variant HPHP::f_dom_element_get_attribute_node(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP32f_dom_element_get_attribute_nodeERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
name => rdx
*/

TypedValue* fh_dom_element_get_attribute_node(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP32f_dom_element_get_attribute_nodeERKNS_7VariantERKNS_6StringE");

/*
HPHP::Variant HPHP::f_dom_element_set_attribute_node(HPHP::Variant const&, HPHP::Object const&)
_ZN4HPHP32f_dom_element_set_attribute_nodeERKNS_7VariantERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
newattr => rdx
*/

TypedValue* fh_dom_element_set_attribute_node(TypedValue* _rv, TypedValue* obj, Value* newattr) asm("_ZN4HPHP32f_dom_element_set_attribute_nodeERKNS_7VariantERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_dom_element_remove_attribute_node(HPHP::Variant const&, HPHP::Object const&)
_ZN4HPHP35f_dom_element_remove_attribute_nodeERKNS_7VariantERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
oldattr => rdx
*/

TypedValue* fh_dom_element_remove_attribute_node(TypedValue* _rv, TypedValue* obj, Value* oldattr) asm("_ZN4HPHP35f_dom_element_remove_attribute_nodeERKNS_7VariantERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_dom_element_get_elements_by_tag_name(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP38f_dom_element_get_elements_by_tag_nameERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
name => rdx
*/

TypedValue* fh_dom_element_get_elements_by_tag_name(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP38f_dom_element_get_elements_by_tag_nameERKNS_7VariantERKNS_6StringE");

/*
HPHP::Variant HPHP::f_dom_element_get_attribute_ns(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP30f_dom_element_get_attribute_nsERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
localname => rcx
*/

TypedValue* fh_dom_element_get_attribute_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* localname) asm("_ZN4HPHP30f_dom_element_get_attribute_nsERKNS_7VariantERKNS_6StringES5_");

/*
HPHP::Variant HPHP::f_dom_element_set_attribute_ns(HPHP::Variant const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP30f_dom_element_set_attribute_nsERKNS_7VariantERKNS_6StringES5_S5_

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
name => rcx
value => r8
*/

TypedValue* fh_dom_element_set_attribute_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* name, Value* value) asm("_ZN4HPHP30f_dom_element_set_attribute_nsERKNS_7VariantERKNS_6StringES5_S5_");

/*
HPHP::Variant HPHP::f_dom_element_remove_attribute_ns(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP33f_dom_element_remove_attribute_nsERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
localname => rcx
*/

TypedValue* fh_dom_element_remove_attribute_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* localname) asm("_ZN4HPHP33f_dom_element_remove_attribute_nsERKNS_7VariantERKNS_6StringES5_");

/*
HPHP::Variant HPHP::f_dom_element_get_attribute_node_ns(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP35f_dom_element_get_attribute_node_nsERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
localname => rcx
*/

TypedValue* fh_dom_element_get_attribute_node_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* localname) asm("_ZN4HPHP35f_dom_element_get_attribute_node_nsERKNS_7VariantERKNS_6StringES5_");

/*
HPHP::Variant HPHP::f_dom_element_set_attribute_node_ns(HPHP::Variant const&, HPHP::Object const&)
_ZN4HPHP35f_dom_element_set_attribute_node_nsERKNS_7VariantERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
newattr => rdx
*/

TypedValue* fh_dom_element_set_attribute_node_ns(TypedValue* _rv, TypedValue* obj, Value* newattr) asm("_ZN4HPHP35f_dom_element_set_attribute_node_nsERKNS_7VariantERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_dom_element_get_elements_by_tag_name_ns(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP41f_dom_element_get_elements_by_tag_name_nsERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
localname => rcx
*/

TypedValue* fh_dom_element_get_elements_by_tag_name_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* localname) asm("_ZN4HPHP41f_dom_element_get_elements_by_tag_name_nsERKNS_7VariantERKNS_6StringES5_");

/*
HPHP::Variant HPHP::f_dom_element_has_attribute(HPHP::Variant const&, HPHP::String const&)
_ZN4HPHP27f_dom_element_has_attributeERKNS_7VariantERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
name => rdx
*/

TypedValue* fh_dom_element_has_attribute(TypedValue* _rv, TypedValue* obj, Value* name) asm("_ZN4HPHP27f_dom_element_has_attributeERKNS_7VariantERKNS_6StringE");

/*
HPHP::Variant HPHP::f_dom_element_has_attribute_ns(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP30f_dom_element_has_attribute_nsERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
localname => rcx
*/

TypedValue* fh_dom_element_has_attribute_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* localname) asm("_ZN4HPHP30f_dom_element_has_attribute_nsERKNS_7VariantERKNS_6StringES5_");

/*
HPHP::Variant HPHP::f_dom_element_set_id_attribute(HPHP::Variant const&, HPHP::String const&, bool)
_ZN4HPHP30f_dom_element_set_id_attributeERKNS_7VariantERKNS_6StringEb

(return value) => rax
_rv => rdi
obj => rsi
name => rdx
isid => rcx
*/

TypedValue* fh_dom_element_set_id_attribute(TypedValue* _rv, TypedValue* obj, Value* name, bool isid) asm("_ZN4HPHP30f_dom_element_set_id_attributeERKNS_7VariantERKNS_6StringEb");

/*
HPHP::Variant HPHP::f_dom_element_set_id_attribute_ns(HPHP::Variant const&, HPHP::String const&, HPHP::String const&, bool)
_ZN4HPHP33f_dom_element_set_id_attribute_nsERKNS_7VariantERKNS_6StringES5_b

(return value) => rax
_rv => rdi
obj => rsi
namespaceuri => rdx
localname => rcx
isid => r8
*/

TypedValue* fh_dom_element_set_id_attribute_ns(TypedValue* _rv, TypedValue* obj, Value* namespaceuri, Value* localname, bool isid) asm("_ZN4HPHP33f_dom_element_set_id_attribute_nsERKNS_7VariantERKNS_6StringES5_b");

/*
HPHP::Variant HPHP::f_dom_element_set_id_attribute_node(HPHP::Variant const&, HPHP::Object const&, bool)
_ZN4HPHP35f_dom_element_set_id_attribute_nodeERKNS_7VariantERKNS_6ObjectEb

(return value) => rax
_rv => rdi
obj => rsi
idattr => rdx
isid => rcx
*/

TypedValue* fh_dom_element_set_id_attribute_node(TypedValue* _rv, TypedValue* obj, Value* idattr, bool isid) asm("_ZN4HPHP35f_dom_element_set_id_attribute_nodeERKNS_7VariantERKNS_6ObjectEb");

/*
HPHP::Variant HPHP::f_dom_text_split_text(HPHP::Variant const&, long long)
_ZN4HPHP21f_dom_text_split_textERKNS_7VariantEx

(return value) => rax
_rv => rdi
obj => rsi
offset => rdx
*/

TypedValue* fh_dom_text_split_text(TypedValue* _rv, TypedValue* obj, long long offset) asm("_ZN4HPHP21f_dom_text_split_textERKNS_7VariantEx");

/*
HPHP::Variant HPHP::f_dom_text_is_whitespace_in_element_content(HPHP::Variant const&)
_ZN4HPHP43f_dom_text_is_whitespace_in_element_contentERKNS_7VariantE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_dom_text_is_whitespace_in_element_content(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP43f_dom_text_is_whitespace_in_element_contentERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_dom_xpath_register_ns(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP23f_dom_xpath_register_nsERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
prefix => rdx
uri => rcx
*/

TypedValue* fh_dom_xpath_register_ns(TypedValue* _rv, TypedValue* obj, Value* prefix, Value* uri) asm("_ZN4HPHP23f_dom_xpath_register_nsERKNS_7VariantERKNS_6StringES5_");

/*
HPHP::Variant HPHP::f_dom_xpath_query(HPHP::Variant const&, HPHP::String const&, HPHP::Object const&)
_ZN4HPHP17f_dom_xpath_queryERKNS_7VariantERKNS_6StringERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
expr => rdx
context => rcx
*/

TypedValue* fh_dom_xpath_query(TypedValue* _rv, TypedValue* obj, Value* expr, Value* context) asm("_ZN4HPHP17f_dom_xpath_queryERKNS_7VariantERKNS_6StringERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_dom_xpath_evaluate(HPHP::Variant const&, HPHP::String const&, HPHP::Object const&)
_ZN4HPHP20f_dom_xpath_evaluateERKNS_7VariantERKNS_6StringERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
expr => rdx
context => rcx
*/

TypedValue* fh_dom_xpath_evaluate(TypedValue* _rv, TypedValue* obj, Value* expr, Value* context) asm("_ZN4HPHP20f_dom_xpath_evaluateERKNS_7VariantERKNS_6StringERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_dom_xpath_register_php_functions(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP34f_dom_xpath_register_php_functionsERKNS_7VariantES2_

(return value) => rax
_rv => rdi
obj => rsi
funcs => rdx
*/

TypedValue* fh_dom_xpath_register_php_functions(TypedValue* _rv, TypedValue* obj, TypedValue* funcs) asm("_ZN4HPHP34f_dom_xpath_register_php_functionsERKNS_7VariantES2_");


} // !HPHP

