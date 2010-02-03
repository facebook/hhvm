/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <cpp/ext/ext_xml.h>
#include "crutch.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Object f_xml_parser_create(CStrRef encoding /* = null_string */) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(encoding), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("xml_parser_create", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

bool f_xml_parser_free(CObjRef parser) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(parser)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("xml_parser_free", _schema, _params);
  return (Variant)_ret[0];
}

int f_xml_parse(CObjRef parser, CStrRef data, bool is_final /* = true */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(parser)), NEW(ArrayElement)(data), NEW(ArrayElement)(is_final), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("xml_parse", _schema, _params);
  return (Variant)_ret[0];
}

int f_xml_parse_into_struct(CObjRef parser, CStrRef data, Variant values, Variant index /* = null */) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(2, "R"), NEW(ArrayElement)(3, "R"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(parser)), NEW(ArrayElement)(data), NEW(ArrayElement)(values), NEW(ArrayElement)(index), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("xml_parse_into_struct", _schema, _params);
  values = ((Variant)_ret[1])[2];
  index = ((Variant)_ret[1])[3];
  return (Variant)_ret[0];
}

Object f_xml_parser_create_ns(CStrRef encoding /* = null_string */, CStrRef separator /* = null_string */) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(encoding), NEW(ArrayElement)(separator), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("xml_parser_create_ns", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

Variant f_xml_parser_get_option(CObjRef parser, int option) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(parser)), NEW(ArrayElement)(option), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("xml_parser_get_option", _schema, _params);
  return (Variant)_ret[0];
}

bool f_xml_parser_set_option(CObjRef parser, int option, CVarRef value) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(parser)), NEW(ArrayElement)(option), NEW(ArrayElement)(value), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("xml_parser_set_option", _schema, _params);
  return (Variant)_ret[0];
}

bool f_xml_set_character_data_handler(CObjRef parser, CStrRef handler) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(parser)), NEW(ArrayElement)(handler), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("xml_set_character_data_handler", _schema, _params);
  return (Variant)_ret[0];
}

bool f_xml_set_default_handler(CObjRef parser, CStrRef handler) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(parser)), NEW(ArrayElement)(handler), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("xml_set_default_handler", _schema, _params);
  return (Variant)_ret[0];
}

bool f_xml_set_element_handler(CObjRef parser, CStrRef start_element_handler, CStrRef end_element_handler) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(parser)), NEW(ArrayElement)(start_element_handler), NEW(ArrayElement)(end_element_handler), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("xml_set_element_handler", _schema, _params);
  return (Variant)_ret[0];
}

bool f_xml_set_processing_instruction_handler(CObjRef parser, CStrRef handler) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(parser)), NEW(ArrayElement)(handler), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("xml_set_processing_instruction_handler", _schema, _params);
  return (Variant)_ret[0];
}

bool f_xml_set_start_namespace_decl_handler(CObjRef parser, CStrRef handler) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(parser)), NEW(ArrayElement)(handler), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("xml_set_start_namespace_decl_handler", _schema, _params);
  return (Variant)_ret[0];
}

bool f_xml_set_end_namespace_decl_handler(CObjRef parser, CStrRef handler) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(parser)), NEW(ArrayElement)(handler), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("xml_set_end_namespace_decl_handler", _schema, _params);
  return (Variant)_ret[0];
}

bool f_xml_set_unparsed_entity_decl_handler(CObjRef parser, CStrRef handler) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(parser)), NEW(ArrayElement)(handler), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("xml_set_unparsed_entity_decl_handler", _schema, _params);
  return (Variant)_ret[0];
}

bool f_xml_set_external_entity_ref_handler(CObjRef parser, CStrRef handler) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(parser)), NEW(ArrayElement)(handler), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("xml_set_external_entity_ref_handler", _schema, _params);
  return (Variant)_ret[0];
}

bool f_xml_set_notation_decl_handler(CObjRef parser, CStrRef handler) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(parser)), NEW(ArrayElement)(handler), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("xml_set_notation_decl_handler", _schema, _params);
  return (Variant)_ret[0];
}

bool f_xml_set_object(CObjRef parser, Variant object) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "R"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(parser)), NEW(ArrayElement)(object), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("xml_set_object", _schema, _params);
  object = ((Variant)_ret[1])[1];
  return (Variant)_ret[0];
}

int f_xml_get_current_byte_index(CObjRef parser) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(parser)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("xml_get_current_byte_index", _schema, _params);
  return (Variant)_ret[0];
}

int f_xml_get_current_column_number(CObjRef parser) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(parser)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("xml_get_current_column_number", _schema, _params);
  return (Variant)_ret[0];
}

int f_xml_get_current_line_number(CObjRef parser) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(parser)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("xml_get_current_line_number", _schema, _params);
  return (Variant)_ret[0];
}

int f_xml_get_error_code(CObjRef parser) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(parser)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("xml_get_error_code", _schema, _params);
  return (Variant)_ret[0];
}

String f_xml_error_string(int code) {
  Array _schema((ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(code), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("xml_error_string", _schema, _params);
  return (Variant)_ret[0];
}

///////////////////////////////////////////////////////////////////////////////

String f_utf8_decode(CStrRef data) {
  char *newbuf = (char*)malloc(data.size() + 1);
  int newlen = 0;
  const char *s = data.data();
  for (int pos = data.size(); pos > 0; ) {
    unsigned short c = (unsigned char)(*s);
    if (c >= 0xf0) { /* four bytes encoded, 21 bits */
      if (pos-4 >= 0) {
        c = ((s[0]&7)<<18) | ((s[1]&63)<<12) | ((s[2]&63)<<6) | (s[3]&63);
      } else {
        c = '?';
      }
      s += 4;
      pos -= 4;
    } else if (c >= 0xe0) { /* three bytes encoded, 16 bits */
      if (pos-3 >= 0) {
        c = ((s[0]&63)<<12) | ((s[1]&63)<<6) | (s[2]&63);
      } else {
        c = '?';
      }
      s += 3;
      pos -= 3;
    } else if (c >= 0xc0) { /* two bytes encoded, 11 bits */
      if (pos-2 >= 0) {
        c = ((s[0]&63)<<6) | (s[1]&63);
      } else {
        c = '?';
      }
      s += 2;
      pos -= 2;
    } else {
      s++;
      pos--;
    }
    newbuf[newlen] = (char)(c > 0xff ? '?' : c);
    ++newlen;
  }
  newbuf[newlen] = '\0';
  return String(newbuf, newlen, AttachString);
}

String f_utf8_encode(CStrRef data) {
  char *newbuf = (char*)malloc(data.size() * 4 + 1);
  int newlen = 0;
  const char *s = data.data();
  for (int pos = data.size(); pos > 0; pos--, s++) {
    unsigned int c = (unsigned char)(*s);
    if (c < 0x80) {
      newbuf[newlen++] = (char) c;
    } else if (c < 0x800) {
      newbuf[newlen++] = (0xc0 | (c >> 6));
      newbuf[newlen++] = (0x80 | (c & 0x3f));
    } else if (c < 0x10000) {
      newbuf[newlen++] = (0xe0 | (c >> 12));
      newbuf[newlen++] = (0xc0 | ((c >> 6) & 0x3f));
      newbuf[newlen++] = (0x80 | (c & 0x3f));
    } else if (c < 0x200000) {
      newbuf[newlen++] = (0xf0 | (c >> 18));
      newbuf[newlen++] = (0xe0 | ((c >> 12) & 0x3f));
      newbuf[newlen++] = (0xc0 | ((c >> 6) & 0x3f));
      newbuf[newlen++] = (0x80 | (c & 0x3f));
    }
  }
  newbuf[newlen] = '\0';
  return String(newbuf, newlen, AttachString);
}

///////////////////////////////////////////////////////////////////////////////
}
