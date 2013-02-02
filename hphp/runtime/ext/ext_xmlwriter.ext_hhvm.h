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
HPHP::Variant HPHP::f_xmlwriter_open_memory()
_ZN4HPHP23f_xmlwriter_open_memoryEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_xmlwriter_open_memory(TypedValue* _rv) asm("_ZN4HPHP23f_xmlwriter_open_memoryEv");

/*
HPHP::Object HPHP::f_xmlwriter_open_uri(HPHP::String const&)
_ZN4HPHP20f_xmlwriter_open_uriERKNS_6StringE

(return value) => rax
_rv => rdi
uri => rsi
*/

Value* fh_xmlwriter_open_uri(Value* _rv, Value* uri) asm("_ZN4HPHP20f_xmlwriter_open_uriERKNS_6StringE");

/*
bool HPHP::f_xmlwriter_set_indent_string(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP29f_xmlwriter_set_indent_stringERKNS_6ObjectERKNS_6StringE

(return value) => rax
xmlwriter => rdi
indentstring => rsi
*/

bool fh_xmlwriter_set_indent_string(Value* xmlwriter, Value* indentstring) asm("_ZN4HPHP29f_xmlwriter_set_indent_stringERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_xmlwriter_set_indent(HPHP::Object const&, bool)
_ZN4HPHP22f_xmlwriter_set_indentERKNS_6ObjectEb

(return value) => rax
xmlwriter => rdi
indent => rsi
*/

bool fh_xmlwriter_set_indent(Value* xmlwriter, bool indent) asm("_ZN4HPHP22f_xmlwriter_set_indentERKNS_6ObjectEb");

/*
bool HPHP::f_xmlwriter_start_document(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP26f_xmlwriter_start_documentERKNS_6ObjectERKNS_6StringES5_S5_

(return value) => rax
xmlwriter => rdi
version => rsi
encoding => rdx
standalone => rcx
*/

bool fh_xmlwriter_start_document(Value* xmlwriter, Value* version, Value* encoding, Value* standalone) asm("_ZN4HPHP26f_xmlwriter_start_documentERKNS_6ObjectERKNS_6StringES5_S5_");

/*
bool HPHP::f_xmlwriter_start_element(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP25f_xmlwriter_start_elementERKNS_6ObjectERKNS_6StringE

(return value) => rax
xmlwriter => rdi
name => rsi
*/

bool fh_xmlwriter_start_element(Value* xmlwriter, Value* name) asm("_ZN4HPHP25f_xmlwriter_start_elementERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_xmlwriter_start_element_ns(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP28f_xmlwriter_start_element_nsERKNS_6ObjectERKNS_6StringES5_S5_

(return value) => rax
xmlwriter => rdi
prefix => rsi
name => rdx
uri => rcx
*/

bool fh_xmlwriter_start_element_ns(Value* xmlwriter, Value* prefix, Value* name, Value* uri) asm("_ZN4HPHP28f_xmlwriter_start_element_nsERKNS_6ObjectERKNS_6StringES5_S5_");

/*
bool HPHP::f_xmlwriter_write_element_ns(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP28f_xmlwriter_write_element_nsERKNS_6ObjectERKNS_6StringES5_S5_S5_

(return value) => rax
xmlwriter => rdi
prefix => rsi
name => rdx
uri => rcx
content => r8
*/

bool fh_xmlwriter_write_element_ns(Value* xmlwriter, Value* prefix, Value* name, Value* uri, Value* content) asm("_ZN4HPHP28f_xmlwriter_write_element_nsERKNS_6ObjectERKNS_6StringES5_S5_S5_");

/*
bool HPHP::f_xmlwriter_write_element(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP25f_xmlwriter_write_elementERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
xmlwriter => rdi
name => rsi
content => rdx
*/

bool fh_xmlwriter_write_element(Value* xmlwriter, Value* name, Value* content) asm("_ZN4HPHP25f_xmlwriter_write_elementERKNS_6ObjectERKNS_6StringES5_");

/*
bool HPHP::f_xmlwriter_end_element(HPHP::Object const&)
_ZN4HPHP23f_xmlwriter_end_elementERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_end_element(Value* xmlwriter) asm("_ZN4HPHP23f_xmlwriter_end_elementERKNS_6ObjectE");

/*
bool HPHP::f_xmlwriter_full_end_element(HPHP::Object const&)
_ZN4HPHP28f_xmlwriter_full_end_elementERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_full_end_element(Value* xmlwriter) asm("_ZN4HPHP28f_xmlwriter_full_end_elementERKNS_6ObjectE");

/*
bool HPHP::f_xmlwriter_start_attribute_ns(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP30f_xmlwriter_start_attribute_nsERKNS_6ObjectERKNS_6StringES5_S5_

(return value) => rax
xmlwriter => rdi
prefix => rsi
name => rdx
uri => rcx
*/

bool fh_xmlwriter_start_attribute_ns(Value* xmlwriter, Value* prefix, Value* name, Value* uri) asm("_ZN4HPHP30f_xmlwriter_start_attribute_nsERKNS_6ObjectERKNS_6StringES5_S5_");

/*
bool HPHP::f_xmlwriter_start_attribute(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP27f_xmlwriter_start_attributeERKNS_6ObjectERKNS_6StringE

(return value) => rax
xmlwriter => rdi
name => rsi
*/

bool fh_xmlwriter_start_attribute(Value* xmlwriter, Value* name) asm("_ZN4HPHP27f_xmlwriter_start_attributeERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_xmlwriter_write_attribute_ns(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP30f_xmlwriter_write_attribute_nsERKNS_6ObjectERKNS_6StringES5_S5_S5_

(return value) => rax
xmlwriter => rdi
prefix => rsi
name => rdx
uri => rcx
content => r8
*/

bool fh_xmlwriter_write_attribute_ns(Value* xmlwriter, Value* prefix, Value* name, Value* uri, Value* content) asm("_ZN4HPHP30f_xmlwriter_write_attribute_nsERKNS_6ObjectERKNS_6StringES5_S5_S5_");

/*
bool HPHP::f_xmlwriter_write_attribute(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP27f_xmlwriter_write_attributeERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
xmlwriter => rdi
name => rsi
value => rdx
*/

bool fh_xmlwriter_write_attribute(Value* xmlwriter, Value* name, Value* value) asm("_ZN4HPHP27f_xmlwriter_write_attributeERKNS_6ObjectERKNS_6StringES5_");

/*
bool HPHP::f_xmlwriter_end_attribute(HPHP::Object const&)
_ZN4HPHP25f_xmlwriter_end_attributeERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_end_attribute(Value* xmlwriter) asm("_ZN4HPHP25f_xmlwriter_end_attributeERKNS_6ObjectE");

/*
bool HPHP::f_xmlwriter_start_cdata(HPHP::Object const&)
_ZN4HPHP23f_xmlwriter_start_cdataERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_start_cdata(Value* xmlwriter) asm("_ZN4HPHP23f_xmlwriter_start_cdataERKNS_6ObjectE");

/*
bool HPHP::f_xmlwriter_write_cdata(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP23f_xmlwriter_write_cdataERKNS_6ObjectERKNS_6StringE

(return value) => rax
xmlwriter => rdi
content => rsi
*/

bool fh_xmlwriter_write_cdata(Value* xmlwriter, Value* content) asm("_ZN4HPHP23f_xmlwriter_write_cdataERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_xmlwriter_end_cdata(HPHP::Object const&)
_ZN4HPHP21f_xmlwriter_end_cdataERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_end_cdata(Value* xmlwriter) asm("_ZN4HPHP21f_xmlwriter_end_cdataERKNS_6ObjectE");

/*
bool HPHP::f_xmlwriter_start_comment(HPHP::Object const&)
_ZN4HPHP25f_xmlwriter_start_commentERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_start_comment(Value* xmlwriter) asm("_ZN4HPHP25f_xmlwriter_start_commentERKNS_6ObjectE");

/*
bool HPHP::f_xmlwriter_write_comment(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP25f_xmlwriter_write_commentERKNS_6ObjectERKNS_6StringE

(return value) => rax
xmlwriter => rdi
content => rsi
*/

bool fh_xmlwriter_write_comment(Value* xmlwriter, Value* content) asm("_ZN4HPHP25f_xmlwriter_write_commentERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_xmlwriter_end_comment(HPHP::Object const&)
_ZN4HPHP23f_xmlwriter_end_commentERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_end_comment(Value* xmlwriter) asm("_ZN4HPHP23f_xmlwriter_end_commentERKNS_6ObjectE");

/*
bool HPHP::f_xmlwriter_end_document(HPHP::Object const&)
_ZN4HPHP24f_xmlwriter_end_documentERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_end_document(Value* xmlwriter) asm("_ZN4HPHP24f_xmlwriter_end_documentERKNS_6ObjectE");

/*
bool HPHP::f_xmlwriter_start_pi(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP20f_xmlwriter_start_piERKNS_6ObjectERKNS_6StringE

(return value) => rax
xmlwriter => rdi
target => rsi
*/

bool fh_xmlwriter_start_pi(Value* xmlwriter, Value* target) asm("_ZN4HPHP20f_xmlwriter_start_piERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_xmlwriter_write_pi(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP20f_xmlwriter_write_piERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
xmlwriter => rdi
target => rsi
content => rdx
*/

bool fh_xmlwriter_write_pi(Value* xmlwriter, Value* target, Value* content) asm("_ZN4HPHP20f_xmlwriter_write_piERKNS_6ObjectERKNS_6StringES5_");

/*
bool HPHP::f_xmlwriter_end_pi(HPHP::Object const&)
_ZN4HPHP18f_xmlwriter_end_piERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_end_pi(Value* xmlwriter) asm("_ZN4HPHP18f_xmlwriter_end_piERKNS_6ObjectE");

/*
bool HPHP::f_xmlwriter_text(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP16f_xmlwriter_textERKNS_6ObjectERKNS_6StringE

(return value) => rax
xmlwriter => rdi
content => rsi
*/

bool fh_xmlwriter_text(Value* xmlwriter, Value* content) asm("_ZN4HPHP16f_xmlwriter_textERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_xmlwriter_write_raw(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP21f_xmlwriter_write_rawERKNS_6ObjectERKNS_6StringE

(return value) => rax
xmlwriter => rdi
content => rsi
*/

bool fh_xmlwriter_write_raw(Value* xmlwriter, Value* content) asm("_ZN4HPHP21f_xmlwriter_write_rawERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_xmlwriter_start_dtd(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP21f_xmlwriter_start_dtdERKNS_6ObjectERKNS_6StringES5_S5_

(return value) => rax
xmlwriter => rdi
qualifiedname => rsi
publicid => rdx
systemid => rcx
*/

bool fh_xmlwriter_start_dtd(Value* xmlwriter, Value* qualifiedname, Value* publicid, Value* systemid) asm("_ZN4HPHP21f_xmlwriter_start_dtdERKNS_6ObjectERKNS_6StringES5_S5_");

/*
bool HPHP::f_xmlwriter_write_dtd(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP21f_xmlwriter_write_dtdERKNS_6ObjectERKNS_6StringES5_S5_S5_

(return value) => rax
xmlwriter => rdi
name => rsi
publicid => rdx
systemid => rcx
subset => r8
*/

bool fh_xmlwriter_write_dtd(Value* xmlwriter, Value* name, Value* publicid, Value* systemid, Value* subset) asm("_ZN4HPHP21f_xmlwriter_write_dtdERKNS_6ObjectERKNS_6StringES5_S5_S5_");

/*
bool HPHP::f_xmlwriter_start_dtd_element(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP29f_xmlwriter_start_dtd_elementERKNS_6ObjectERKNS_6StringE

(return value) => rax
xmlwriter => rdi
qualifiedname => rsi
*/

bool fh_xmlwriter_start_dtd_element(Value* xmlwriter, Value* qualifiedname) asm("_ZN4HPHP29f_xmlwriter_start_dtd_elementERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_xmlwriter_write_dtd_element(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP29f_xmlwriter_write_dtd_elementERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
xmlwriter => rdi
name => rsi
content => rdx
*/

bool fh_xmlwriter_write_dtd_element(Value* xmlwriter, Value* name, Value* content) asm("_ZN4HPHP29f_xmlwriter_write_dtd_elementERKNS_6ObjectERKNS_6StringES5_");

/*
bool HPHP::f_xmlwriter_end_dtd_element(HPHP::Object const&)
_ZN4HPHP27f_xmlwriter_end_dtd_elementERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_end_dtd_element(Value* xmlwriter) asm("_ZN4HPHP27f_xmlwriter_end_dtd_elementERKNS_6ObjectE");

/*
bool HPHP::f_xmlwriter_start_dtd_attlist(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP29f_xmlwriter_start_dtd_attlistERKNS_6ObjectERKNS_6StringE

(return value) => rax
xmlwriter => rdi
name => rsi
*/

bool fh_xmlwriter_start_dtd_attlist(Value* xmlwriter, Value* name) asm("_ZN4HPHP29f_xmlwriter_start_dtd_attlistERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_xmlwriter_write_dtd_attlist(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP29f_xmlwriter_write_dtd_attlistERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
xmlwriter => rdi
name => rsi
content => rdx
*/

bool fh_xmlwriter_write_dtd_attlist(Value* xmlwriter, Value* name, Value* content) asm("_ZN4HPHP29f_xmlwriter_write_dtd_attlistERKNS_6ObjectERKNS_6StringES5_");

/*
bool HPHP::f_xmlwriter_end_dtd_attlist(HPHP::Object const&)
_ZN4HPHP27f_xmlwriter_end_dtd_attlistERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_end_dtd_attlist(Value* xmlwriter) asm("_ZN4HPHP27f_xmlwriter_end_dtd_attlistERKNS_6ObjectE");

/*
bool HPHP::f_xmlwriter_start_dtd_entity(HPHP::Object const&, HPHP::String const&, bool)
_ZN4HPHP28f_xmlwriter_start_dtd_entityERKNS_6ObjectERKNS_6StringEb

(return value) => rax
xmlwriter => rdi
name => rsi
isparam => rdx
*/

bool fh_xmlwriter_start_dtd_entity(Value* xmlwriter, Value* name, bool isparam) asm("_ZN4HPHP28f_xmlwriter_start_dtd_entityERKNS_6ObjectERKNS_6StringEb");

/*
bool HPHP::f_xmlwriter_write_dtd_entity(HPHP::Object const&, HPHP::String const&, HPHP::String const&, bool, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP28f_xmlwriter_write_dtd_entityERKNS_6ObjectERKNS_6StringES5_bS5_S5_S5_

(return value) => rax
xmlwriter => rdi
name => rsi
content => rdx
pe => rcx
publicid => r8
systemid => r9
ndataid => st0
*/

bool fh_xmlwriter_write_dtd_entity(Value* xmlwriter, Value* name, Value* content, bool pe, Value* publicid, Value* systemid, Value* ndataid) asm("_ZN4HPHP28f_xmlwriter_write_dtd_entityERKNS_6ObjectERKNS_6StringES5_bS5_S5_S5_");

/*
bool HPHP::f_xmlwriter_end_dtd_entity(HPHP::Object const&)
_ZN4HPHP26f_xmlwriter_end_dtd_entityERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_end_dtd_entity(Value* xmlwriter) asm("_ZN4HPHP26f_xmlwriter_end_dtd_entityERKNS_6ObjectE");

/*
bool HPHP::f_xmlwriter_end_dtd(HPHP::Object const&)
_ZN4HPHP19f_xmlwriter_end_dtdERKNS_6ObjectE

(return value) => rax
xmlwriter => rdi
*/

bool fh_xmlwriter_end_dtd(Value* xmlwriter) asm("_ZN4HPHP19f_xmlwriter_end_dtdERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_xmlwriter_flush(HPHP::Object const&, bool)
_ZN4HPHP17f_xmlwriter_flushERKNS_6ObjectEb

(return value) => rax
_rv => rdi
xmlwriter => rsi
empty => rdx
*/

TypedValue* fh_xmlwriter_flush(TypedValue* _rv, Value* xmlwriter, bool empty) asm("_ZN4HPHP17f_xmlwriter_flushERKNS_6ObjectEb");

/*
HPHP::String HPHP::f_xmlwriter_output_memory(HPHP::Object const&, bool)
_ZN4HPHP25f_xmlwriter_output_memoryERKNS_6ObjectEb

(return value) => rax
_rv => rdi
xmlwriter => rsi
flush => rdx
*/

Value* fh_xmlwriter_output_memory(Value* _rv, Value* xmlwriter, bool flush) asm("_ZN4HPHP25f_xmlwriter_output_memoryERKNS_6ObjectEb");


} // !HPHP

