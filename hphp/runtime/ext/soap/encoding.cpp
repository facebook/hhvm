/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/soap/encoding.h"
#include "hphp/runtime/ext/soap/soap.h"
#include "hphp/runtime/ext/ext_soap.h"
#include "hphp/runtime/ext/ext_string.h"
#include "hphp/runtime/base/string-buffer.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool php_libxml_xmlCheckUTF8(const unsigned char *s) {
  int i;
  unsigned char c;

  for (i = 0; (c = s[i++]);) {
    if ((c & 0x80) == 0) {
    } else if ((c & 0xe0) == 0xc0) {
      if ((s[i++] & 0xc0) != 0x80) {
        return false;
      }
    } else if ((c & 0xf0) == 0xe0) {
      if ((s[i++] & 0xc0) != 0x80 || (s[i++] & 0xc0) != 0x80) {
        return false;
      }
    } else if ((c & 0xf8) == 0xf0) {
      if ((s[i++] & 0xc0) != 0x80 || (s[i++] & 0xc0) != 0x80 ||
          (s[i++] & 0xc0) != 0x80) {
        return false;
      }
    } else {
      return false;
    }
  }
  return true;
}

static int zend_strnlen(const char* s, int maxlen) {
  int len = 0;
  while (*s++ && maxlen--) len++;
  return len;
}

static bool zend_unmangle_property_name(const char *mangled_property, int len,
                                        const char **class_name,
                                        const char **prop_name) {
  int class_name_len;
  *class_name = NULL;

  if (mangled_property[0]!=0) {
    *prop_name = mangled_property;
    return true;
  }
  if (len < 3 || mangled_property[1]==0) {
    raise_warning("Illegal member variable name");
    *prop_name = mangled_property;
    return false;
  }

  class_name_len = zend_strnlen(mangled_property+1, --len - 1) + 1;
  if (class_name_len >= len || mangled_property[class_name_len]!=0) {
    raise_warning("Corrupt member variable name");
    *prop_name = mangled_property;
    return false;
  }
  *class_name = mangled_property+1;
  *prop_name = (*class_name)+class_name_len;
  return true;
}

///////////////////////////////////////////////////////////////////////////////

static Variant to_zval_double  (encodeTypePtr type, xmlNodePtr data);
static Variant to_zval_long    (encodeTypePtr type, xmlNodePtr data);
static Variant to_zval_bool    (encodeTypePtr type, xmlNodePtr data);
static Variant to_zval_string  (encodeTypePtr type, xmlNodePtr data);
static Variant to_zval_stringr (encodeTypePtr type, xmlNodePtr data);
static Variant to_zval_stringc (encodeTypePtr type, xmlNodePtr data);
static Variant to_zval_map     (encodeTypePtr type, xmlNodePtr data);
static Variant to_zval_null    (encodeTypePtr type, xmlNodePtr data);
static Variant to_zval_base64  (encodeTypePtr type, xmlNodePtr data);
static Variant to_zval_hexbin  (encodeTypePtr type, xmlNodePtr data);

static xmlNodePtr to_xml_long
(encodeTypePtr type, CVarRef data, int style, xmlNodePtr parent);
static xmlNodePtr to_xml_double
(encodeTypePtr type, CVarRef data, int style, xmlNodePtr parent);
static xmlNodePtr to_xml_bool
(encodeTypePtr type, CVarRef data, int style, xmlNodePtr parent);

/* String encode */
static xmlNodePtr to_xml_string
(encodeTypePtr type, CVarRef data, int style, xmlNodePtr parent);
static xmlNodePtr to_xml_base64
(encodeTypePtr type, CVarRef data, int style, xmlNodePtr parent);
static xmlNodePtr to_xml_hexbin
(encodeTypePtr type, CVarRef data, int style, xmlNodePtr parent);

/* Null encode */
static xmlNodePtr to_xml_null
(encodeTypePtr type, CVarRef data, int style, xmlNodePtr parent);

/* Array encode */
static xmlNodePtr guess_array_map
(encodeTypePtr type, CVarRef data, int style, xmlNodePtr parent);
static xmlNodePtr to_xml_map
(encodeTypePtr type, CVarRef data, int style, xmlNodePtr parent);

static xmlNodePtr to_xml_list
(encodeTypePtr enc, CVarRef data, int style, xmlNodePtr parent);
static xmlNodePtr to_xml_list1
(encodeTypePtr enc, CVarRef data, int style, xmlNodePtr parent);

/* Datetime encode/decode */
static xmlNodePtr to_xml_datetime_ex
(encodeTypePtr type, CVarRef data, const char *format, int style,
 xmlNodePtr parent);
static xmlNodePtr to_xml_datetime
(encodeTypePtr type, CVarRef data, int style, xmlNodePtr parent);
static xmlNodePtr to_xml_time
(encodeTypePtr type, CVarRef data, int style, xmlNodePtr parent);
static xmlNodePtr to_xml_date
(encodeTypePtr type, CVarRef data, int style, xmlNodePtr parent);
static xmlNodePtr to_xml_gyearmonth
(encodeTypePtr type, CVarRef data, int style, xmlNodePtr parent);
static xmlNodePtr to_xml_gyear
(encodeTypePtr type, CVarRef data, int style, xmlNodePtr parent);
static xmlNodePtr to_xml_gmonthday
(encodeTypePtr type, CVarRef data, int style, xmlNodePtr parent);
static xmlNodePtr to_xml_gday
(encodeTypePtr type, CVarRef data, int style, xmlNodePtr parent);
static xmlNodePtr to_xml_gmonth
(encodeTypePtr type, CVarRef data, int style, xmlNodePtr parent);
static xmlNodePtr to_xml_duration
(encodeTypePtr type, CVarRef data, int style, xmlNodePtr parent);

static Variant to_zval_object(encodeTypePtr type, xmlNodePtr data);
static Variant to_zval_array(encodeTypePtr type, xmlNodePtr data);

static xmlNodePtr to_xml_object
(encodeTypePtr type, CVarRef data, int style, xmlNodePtr parent);
static xmlNodePtr to_xml_array
(encodeTypePtr type, CVarRef data, int style, xmlNodePtr parent);

static Variant to_zval_any
(encodeTypePtr type, xmlNodePtr data);
static xmlNodePtr to_xml_any
(encodeTypePtr type, CVarRef data, int style, xmlNodePtr parent);

/* Try and guess for non-wsdl clients and servers */
static Variant guess_zval_convert
(encodeTypePtr type, xmlNodePtr data);
static xmlNodePtr guess_xml_convert
(encodeTypePtr type, CVarRef data, int style, xmlNodePtr parent);

static encodePtr get_array_type
(xmlNodePtr node, CVarRef array, string &out_type);

static xmlNodePtr check_and_resolve_href(xmlNodePtr data);

static void set_ns_prop(xmlNodePtr node, const char *ns, const char *name,
                        const char *val);
static void set_xsi_nil(xmlNodePtr node);
static void set_xsi_type(xmlNodePtr node, const char *type);

static void get_type_str
(xmlNodePtr node, const char* ns, const char* type, string &ret);
static void set_ns_and_type_ex(xmlNodePtr node, const char *ns,
                               const char *type);

static void set_ns_and_type(xmlNodePtr node, encodeTypePtr type);

#define FIND_XML_NULL(xml, v)                                   \
  {                                                             \
    if (!xml) {                                                 \
      v.reset();                                                \
      return v;                                                 \
    }                                                           \
    if (xml->properties) {                                      \
      xmlAttrPtr n = get_attribute(xml->properties, "nil");     \
      if (n) {                                                  \
        v.reset();                                              \
        return v;                                               \
      }                                                         \
    }                                                           \
  }

#define FIND_ZVAL_NULL(v, xml, style)                           \
  {                                                             \
    if (v.isNull()) {                                           \
      if (style == SOAP_ENCODED) {                              \
        set_xsi_nil(xml);                                       \
      }                                                         \
      return xml;                                               \
    }                                                           \
  }

///////////////////////////////////////////////////////////////////////////////

encodeStatic s_defaultEncoding[] = {
  {UNKNOWN_TYPE, "", "", guess_zval_convert, guess_xml_convert},

  {KindOfUninit, "nil", XSI_NAMESPACE,
   to_zval_null, to_xml_null},
  {KindOfNull, "nil", XSI_NAMESPACE,
   to_zval_null, to_xml_null},
  {KindOfBoolean, XSD_BOOLEAN_STRING, XSD_NAMESPACE,
   to_zval_bool, to_xml_bool},
  {KindOfInt64, XSD_INT_STRING, XSD_NAMESPACE,
   to_zval_long, to_xml_long},
  {KindOfDouble, XSD_FLOAT_STRING, XSD_NAMESPACE,
   to_zval_double, to_xml_double},
  {KindOfStaticString, XSD_STRING_STRING, XSD_NAMESPACE,
   to_zval_string, to_xml_string},
  {KindOfString, XSD_STRING_STRING, XSD_NAMESPACE,
   to_zval_string, to_xml_string},
  {KindOfArray, SOAP_ENC_ARRAY_STRING, SOAP_1_1_ENC_NAMESPACE,
   to_zval_array, guess_array_map},
  {KindOfObject, SOAP_ENC_OBJECT_STRING, SOAP_1_1_ENC_NAMESPACE,
   to_zval_object, to_xml_object},
  {KindOfArray, SOAP_ENC_ARRAY_STRING, SOAP_1_2_ENC_NAMESPACE,
   to_zval_array, guess_array_map},
  {KindOfObject, SOAP_ENC_OBJECT_STRING, SOAP_1_2_ENC_NAMESPACE,
   to_zval_object, to_xml_object},

  {XSD_STRING, XSD_STRING_STRING, XSD_NAMESPACE,
   to_zval_string, to_xml_string},
  {XSD_BOOLEAN, XSD_BOOLEAN_STRING, XSD_NAMESPACE,
   to_zval_bool, to_xml_bool},
  {XSD_DECIMAL, XSD_DECIMAL_STRING, XSD_NAMESPACE,
   to_zval_stringc, to_xml_string},
  {XSD_FLOAT, XSD_FLOAT_STRING, XSD_NAMESPACE,
   to_zval_double, to_xml_double},
  {XSD_DOUBLE, XSD_DOUBLE_STRING, XSD_NAMESPACE,
   to_zval_double, to_xml_double},

  {XSD_DATETIME, XSD_DATETIME_STRING, XSD_NAMESPACE,
   to_zval_stringc, to_xml_datetime},
  {XSD_TIME, XSD_TIME_STRING, XSD_NAMESPACE,
   to_zval_stringc, to_xml_time},
  {XSD_DATE, XSD_DATE_STRING, XSD_NAMESPACE,
   to_zval_stringc, to_xml_date},
  {XSD_GYEARMONTH, XSD_GYEARMONTH_STRING, XSD_NAMESPACE,
   to_zval_stringc, to_xml_gyearmonth},
  {XSD_GYEAR, XSD_GYEAR_STRING, XSD_NAMESPACE,
   to_zval_stringc, to_xml_gyear},
  {XSD_GMONTHDAY, XSD_GMONTHDAY_STRING, XSD_NAMESPACE,
   to_zval_stringc, to_xml_gmonthday},
  {XSD_GDAY, XSD_GDAY_STRING, XSD_NAMESPACE,
   to_zval_stringc, to_xml_gday},
  {XSD_GMONTH, XSD_GMONTH_STRING, XSD_NAMESPACE,
   to_zval_stringc, to_xml_gmonth},
  {XSD_DURATION, XSD_DURATION_STRING, XSD_NAMESPACE,
   to_zval_stringc, to_xml_duration},

  {XSD_HEXBINARY, XSD_HEXBINARY_STRING, XSD_NAMESPACE,
   to_zval_hexbin, to_xml_hexbin},
  {XSD_BASE64BINARY, XSD_BASE64BINARY_STRING, XSD_NAMESPACE,
   to_zval_base64, to_xml_base64},

  {XSD_LONG, XSD_LONG_STRING, XSD_NAMESPACE,
   to_zval_long, to_xml_long},
  {XSD_INT, XSD_INT_STRING, XSD_NAMESPACE,
   to_zval_long, to_xml_long},
  {XSD_SHORT, XSD_SHORT_STRING, XSD_NAMESPACE,
   to_zval_long, to_xml_long},
  {XSD_BYTE, XSD_BYTE_STRING, XSD_NAMESPACE,
   to_zval_long, to_xml_long},
  {XSD_NONPOSITIVEINTEGER,XSD_NONPOSITIVEINTEGER_STRING,XSD_NAMESPACE,
   to_zval_long, to_xml_long},
  {XSD_POSITIVEINTEGER, XSD_POSITIVEINTEGER_STRING, XSD_NAMESPACE,
   to_zval_long, to_xml_long},
  {XSD_NONNEGATIVEINTEGER,XSD_NONNEGATIVEINTEGER_STRING,XSD_NAMESPACE,
   to_zval_long, to_xml_long},
  {XSD_NEGATIVEINTEGER, XSD_NEGATIVEINTEGER_STRING, XSD_NAMESPACE,
   to_zval_long, to_xml_long},
  {XSD_UNSIGNEDBYTE, XSD_UNSIGNEDBYTE_STRING, XSD_NAMESPACE,
   to_zval_long, to_xml_long},
  {XSD_UNSIGNEDSHORT, XSD_UNSIGNEDSHORT_STRING, XSD_NAMESPACE,
   to_zval_long, to_xml_long},
  {XSD_UNSIGNEDINT, XSD_UNSIGNEDINT_STRING, XSD_NAMESPACE,
   to_zval_long, to_xml_long},
  {XSD_UNSIGNEDLONG, XSD_UNSIGNEDLONG_STRING, XSD_NAMESPACE,
   to_zval_long, to_xml_long},
  {XSD_INTEGER, XSD_INTEGER_STRING, XSD_NAMESPACE,
   to_zval_long, to_xml_long},

  {XSD_ANYTYPE, XSD_ANYTYPE_STRING, XSD_NAMESPACE,
   guess_zval_convert, guess_xml_convert},
  {XSD_UR_TYPE, XSD_UR_TYPE_STRING, XSD_NAMESPACE,
   guess_zval_convert, guess_xml_convert},
  {XSD_ANYURI, XSD_ANYURI_STRING, XSD_NAMESPACE,
   to_zval_stringc, to_xml_string},
  {XSD_QNAME, XSD_QNAME_STRING, XSD_NAMESPACE,
   to_zval_stringc, to_xml_string},
  {XSD_NOTATION, XSD_NOTATION_STRING, XSD_NAMESPACE,
   to_zval_stringc, to_xml_string},
  {XSD_NORMALIZEDSTRING, XSD_NORMALIZEDSTRING_STRING, XSD_NAMESPACE,
   to_zval_stringr, to_xml_string},
  {XSD_TOKEN, XSD_TOKEN_STRING, XSD_NAMESPACE,
   to_zval_stringc, to_xml_string},
  {XSD_LANGUAGE, XSD_LANGUAGE_STRING, XSD_NAMESPACE,
   to_zval_stringc, to_xml_string},
  {XSD_NMTOKEN, XSD_NMTOKEN_STRING, XSD_NAMESPACE,
   to_zval_stringc, to_xml_string},
  {XSD_NMTOKENS, XSD_NMTOKENS_STRING, XSD_NAMESPACE,
   to_zval_stringc, to_xml_list1},
  {XSD_NAME, XSD_NAME_STRING, XSD_NAMESPACE,
   to_zval_stringc, to_xml_string},
  {XSD_NCNAME, XSD_NCNAME_STRING, XSD_NAMESPACE,
   to_zval_stringc, to_xml_string},
  {XSD_ID, XSD_ID_STRING, XSD_NAMESPACE,
   to_zval_stringc, to_xml_string},
  {XSD_IDREF, XSD_IDREF_STRING, XSD_NAMESPACE,
   to_zval_stringc, to_xml_string},
  {XSD_IDREFS, XSD_IDREFS_STRING, XSD_NAMESPACE,
   to_zval_stringc, to_xml_list1},
  {XSD_ENTITY, XSD_ENTITY_STRING, XSD_NAMESPACE,
   to_zval_stringc, to_xml_string},
  {XSD_ENTITIES, XSD_ENTITIES_STRING, XSD_NAMESPACE,
   to_zval_stringc, to_xml_list1},

  {APACHE_MAP, APACHE_MAP_STRING, APACHE_NAMESPACE,
   to_zval_map, to_xml_map},

  {SOAP_ENC_OBJECT, SOAP_ENC_OBJECT_STRING, SOAP_1_1_ENC_NAMESPACE,
   to_zval_object, to_xml_object},
  {SOAP_ENC_ARRAY, SOAP_ENC_ARRAY_STRING, SOAP_1_1_ENC_NAMESPACE,
   to_zval_array, to_xml_array},
  {SOAP_ENC_OBJECT, SOAP_ENC_OBJECT_STRING, SOAP_1_2_ENC_NAMESPACE,
   to_zval_object, to_xml_object},
  {SOAP_ENC_ARRAY, SOAP_ENC_ARRAY_STRING, SOAP_1_2_ENC_NAMESPACE,
   to_zval_array, to_xml_array},

  /* support some of the 1999 data types */
  {XSD_STRING, XSD_STRING_STRING, XSD_1999_NAMESPACE,
   to_zval_string, to_xml_string},
  {XSD_BOOLEAN, XSD_BOOLEAN_STRING, XSD_1999_NAMESPACE,
   to_zval_bool, to_xml_bool},
  {XSD_DECIMAL, XSD_DECIMAL_STRING, XSD_1999_NAMESPACE,
   to_zval_stringc, to_xml_string},
  {XSD_FLOAT, XSD_FLOAT_STRING, XSD_1999_NAMESPACE,
   to_zval_double, to_xml_double},
  {XSD_DOUBLE, XSD_DOUBLE_STRING, XSD_1999_NAMESPACE,
   to_zval_double, to_xml_double},

  {XSD_LONG, XSD_LONG_STRING, XSD_1999_NAMESPACE,
   to_zval_long, to_xml_long},
  {XSD_INT, XSD_INT_STRING, XSD_1999_NAMESPACE,
   to_zval_long, to_xml_long},
  {XSD_SHORT, XSD_SHORT_STRING, XSD_1999_NAMESPACE,
   to_zval_long, to_xml_long},
  {XSD_BYTE, XSD_BYTE_STRING, XSD_1999_NAMESPACE,
   to_zval_long, to_xml_long},
  {XSD_1999_TIMEINSTANT,XSD_1999_TIMEINSTANT_STRING,XSD_1999_NAMESPACE,
   to_zval_stringc, to_xml_string},

  {XSD_ANYXML, "<anyXML>", "<anyXML>", to_zval_any, to_xml_any},

  {END_KNOWN_TYPES, NULL, NULL, guess_zval_convert, guess_xml_convert},
};

///////////////////////////////////////////////////////////////////////////////

static string get_full_typename(const char *ns, const char *type) {
  string nscat;
  if (ns && *ns) {
    nscat = ns;
    nscat += ':';
  }
  if (type) {
    nscat += type;
  }
  return nscat;
}

static encodePtr get_typemap_type(const char *ns, const char *type) {
  USE_SOAP_GLOBAL;
  string nscat = get_full_typename(ns, type);
  if (SOAP_GLOBAL(typemap)) {
    encodeMap::const_iterator iter = SOAP_GLOBAL(typemap)->find(nscat);
    if (iter != SOAP_GLOBAL(typemap)->end()) {
      return iter->second;
    }
  }
  return encodePtr();
}

void whiteSpace_replace(xmlChar* str) {
  while (*str != '\0') {
    if (*str == '\x9' || *str == '\xA' || *str == '\xD') {
      *str = ' ';
    }
    str++;
  }
}

void whiteSpace_collapse(xmlChar* str) {
  xmlChar *pos;
  xmlChar old;

  pos = str;
  whiteSpace_replace(str);
  while (*str == ' ') {
    str++;
  }
  old = '\0';
  while (*str != '\0') {
    if (*str != ' ' || old != ' ') {
      *pos = *str;
      pos++;
    }
    old = *str;
    str++;
  }
  if (old == ' ') {
     --pos;
  }
  *pos = '\0';
}

static encodePtr find_encoder_by_type_name(sdl *sdl, const char *type) {
  if (sdl) {
    encodeMap::const_iterator iter = sdl->encoders.find(type);
    if (iter != sdl->encoders.end()) {
      return iter->second;
    }
  }
  return encodePtr();
}

static bool soap_check_zval_ref(CVarRef data, xmlNodePtr node) {
  USE_SOAP_GLOBAL;
  int64_t hash = 0;
  if (data.isObject()) {
    hash = (int64_t)data.getObjectData();
  } else if (data.isReferenced()) {
    hash = (int64_t)data.getRefData();
  }
  if (hash) {
    Array &ref_map = SOAP_GLOBAL(ref_map);
    xmlNodePtr node_ptr = (xmlNodePtr)ref_map[hash].toInt64();
    if (node_ptr) {
      if (node_ptr == node) {
        return false;
      }
      xmlNodeSetName(node, node_ptr->name);
      xmlSetNs(node, node_ptr->ns);
      xmlAttrPtr attr = node_ptr->properties;
      const char *id;
      string prefix;
      if (SOAP_GLOBAL(soap_version) == SOAP_1_1) {
        while (1) {
          attr = get_attribute(attr, "id");
          if (attr == NULL || attr->ns == NULL) {
            break;
          }
          attr = attr->next;
        }
        if (attr) {
          prefix = '#';
          prefix += (char*)attr->children->content;
          id = prefix.c_str();
        } else {
          SOAP_GLOBAL(cur_uniq_ref)++;
          prefix = "#ref";
          prefix += lexical_cast<string>(SOAP_GLOBAL(cur_uniq_ref));
          id = prefix.c_str();
          xmlSetProp(node_ptr, BAD_CAST("id"), BAD_CAST(id+1));
        }
        xmlSetProp(node, BAD_CAST("href"), BAD_CAST(id));
      } else {
        attr = get_attribute_ex(attr, "id", SOAP_1_2_ENC_NAMESPACE);
        if (attr) {
          prefix = '#';
          prefix += (char*)attr->children->content;
          id = prefix.c_str();
        } else {
          SOAP_GLOBAL(cur_uniq_ref)++;
          prefix = "#ref";
          prefix += lexical_cast<string>(SOAP_GLOBAL(cur_uniq_ref));
          id = prefix.c_str();
          set_ns_prop(node_ptr, SOAP_1_2_ENC_NAMESPACE, "id", id+1);
        }
        set_ns_prop(node, SOAP_1_2_ENC_NAMESPACE, "ref", id);
      }
      return true;
    }
    ref_map.set(hash, (int64_t)node);
  }
  return false;
}

static bool soap_check_xml_ref(Variant &data, xmlNodePtr node) {
  USE_SOAP_GLOBAL;
  Array &ref_map = SOAP_GLOBAL(ref_map);
  if (ref_map.exists((int64_t)node)) {
    Variant &data2 = ref_map.lvalAt((int64_t)node);
    if (!(data.isObject() && data2.isObject() &&
          data.getObjectData() == data2.getObjectData()) &&
        !(data.isReferenced() && data2.isReferenced() &&
          data.getRefData() == data2.getRefData())) {
      data.assignRef(data2);
      return true;
    }
  } else {
    ref_map.set((int64_t)node, ref(data));
  }
  return false;
}

static xmlNodePtr master_to_xml_int(encodePtr encode, CVarRef data, int style,
                                    xmlNodePtr parent, bool check_class_map) {
  USE_SOAP_GLOBAL;
  xmlNodePtr node = NULL;
  bool add_type = false;

  /* Special handling of class SoapVar */
  if (data.isObject() && data.toObject().instanceof(c_SoapVar::classof())) {
    encodePtr enc;
    c_SoapVar *p = data.toObject().getTyped<c_SoapVar>();
    if (!p->m_ns.empty()) {
      enc = get_encoder(SOAP_GLOBAL(sdl), p->m_ns.data(), p->m_stype.data());
    } else {
      enc = get_encoder_ex(SOAP_GLOBAL(sdl), p->m_stype.data());
    }
    if (!enc && SOAP_GLOBAL(typemap)) {
      enc = get_typemap_type(p->m_ns.c_str(), p->m_stype.c_str());
    }
    if (!enc) {
      enc = get_conversion(p->m_type);
    }
    if (!enc) {
      enc = encode;
    }

    node = master_to_xml(enc, p->m_value, style, parent);

    if (style == SOAP_ENCODED || (SOAP_GLOBAL(sdl) && encode != enc)) {
      if (!p->m_stype.empty()) {
        set_ns_and_type_ex(node, p->m_ns.c_str(), p->m_stype.c_str());
      }
    }

    if (!p->m_name.empty()) {
      xmlNodeSetName(node, BAD_CAST(p->m_name.data()));
    }
    if (!p->m_namens.empty()) {
      xmlNsPtr nsp = encode_add_ns(node, p->m_namens.data());
      xmlSetNs(node, nsp);
    }
  } else {
    if (check_class_map && !SOAP_GLOBAL(classmap).empty() &&
        data.isString() && !data.toString().empty()) {
      String clsname = data.toString();
      for (ArrayIter iter(SOAP_GLOBAL(classmap)); iter; ++iter) {
        if (same(iter.second(), clsname)) {
          /* TODO: namespace isn't stored */
          encodePtr enc;
          if (SOAP_GLOBAL(sdl)) {
            enc = get_encoder(SOAP_GLOBAL(sdl),
                              SOAP_GLOBAL(sdl)->target_ns.c_str(),
                              iter.first().toString().data());
            if (!enc) {
              enc = find_encoder_by_type_name(SOAP_GLOBAL(sdl),
                                              iter.first().toString().data());
            }
          }
          if (enc) {
            if (encode != enc && style == SOAP_LITERAL) {
              add_type = true;
            }
            encode = enc;
          }
          break;
        }
      }
    }

    if (encode == NULL) {
      encode = get_conversion(UNKNOWN_TYPE);
    }
    if (SOAP_GLOBAL(typemap) && !encode->details.type_str.empty()) {
      encodePtr enc = get_typemap_type(encode->details.ns.c_str(),
                                       encode->details.type_str.c_str());
      if (enc) encode = enc;
    }
    if (encode->to_xml) {
      node = encode->to_xml(&encode->details, data, style, parent);
      if (add_type) {
        set_ns_and_type(node, &encode->details);
      }
    }
  }
  return node;
}

xmlNodePtr master_to_xml(encodePtr encode, CVarRef data, int style,
                         xmlNodePtr parent) {
  return master_to_xml_int(encode, data, style, parent, true);
}

static Variant master_to_zval_int(encodePtr encode, xmlNodePtr data) {
  USE_SOAP_GLOBAL;
  Variant ret;
  if (SOAP_GLOBAL(typemap)) {
    if (!encode->details.type_str.empty()) {
      encodePtr enc = get_typemap_type(encode->details.ns.c_str(),
                                       encode->details.type_str.c_str());
      if (enc) encode = enc;
    } else {
      xmlAttrPtr type_attr = get_attribute_ex(data->properties, "type",
                                              XSI_NAMESPACE);
      if (type_attr) {
        string ns, cptype;
        parse_namespace(type_attr->children->content, cptype, ns);
        xmlNsPtr nsptr = xmlSearchNs(data->doc, data, NS_STRING(ns));
        string nscat;
        if (nsptr) {
          nscat = (char*)nsptr->href;
          nscat += ':';
        }
        nscat += cptype.data();
        encodeMap::const_iterator iter = SOAP_GLOBAL(typemap)->find(nscat);
        if (iter != SOAP_GLOBAL(typemap)->end()) {
          encode = iter->second;
        }
      }
    }
  }
  if (encode->to_zval) {
    ret = encode->to_zval(&encode->details, data);
  }
  return ret;
}

Variant master_to_zval(encodePtr encode, xmlNodePtr data) {
  USE_SOAP_GLOBAL;
  data = check_and_resolve_href(data);

  if (encode == NULL) {
    encode = get_conversion(UNKNOWN_TYPE);
  } else {
    /* Use xsi:type if it is defined */
    xmlAttrPtr type_attr = get_attribute_ex(data->properties, "type",
                                            XSI_NAMESPACE);
    if (type_attr) {
      encodePtr enc = get_encoder_from_prefix(SOAP_GLOBAL(sdl), data,
                                              type_attr->children->content);
      if (enc && enc != encode) {
        encodePtr tmp = enc;
        while (tmp && tmp->details.sdl_type &&
               tmp->details.sdl_type->kind != XSD_TYPEKIND_COMPLEX) {
          if (enc == tmp->details.sdl_type->encode ||
              tmp == tmp->details.sdl_type->encode) {
            enc.reset();
            break;
          }
          tmp = tmp->details.sdl_type->encode;
        }
        if (enc) {
          encode = enc;
        }
      }
    }
  }
  return master_to_zval_int(encode, data);
}

xmlNodePtr to_xml_user(encodeTypePtr type, CVarRef data, int style,
                       xmlNodePtr parent) {
  xmlNodePtr ret = NULL;
  if (type && type->map && !type->map->to_xml.isNull()) {
    Variant return_value = vm_call_user_func(type->map->to_xml,
                                                  make_packed_array(data));
    if (return_value.isString()) {
      String sdoc = return_value.toString();
      xmlDocPtr doc = soap_xmlParseMemory(sdoc.data(), sdoc.size());
      if (doc && doc->children) {
        ret = xmlDocCopyNode(doc->children, parent->doc, 1);
      }
      xmlFreeDoc(doc);
    }
  }
  if (!ret) {
    ret = xmlNewNode(NULL, BAD_CAST("BOGUS"));
  }
  xmlAddChild(parent, ret);
  if (style == SOAP_ENCODED) {
    set_ns_and_type(ret, type);
  }
  return ret;
}

Variant to_zval_user(encodeTypePtr type, xmlNodePtr node) {
  Variant return_value;
  if (type && type->map && !type->map->to_zval.isNull()) {
    xmlNodePtr copy = xmlCopyNode(node, 1);
    xmlBufferPtr buf = xmlBufferCreate();
    xmlNodeDump(buf, NULL, copy, 0, 0);
    String data((char*)xmlBufferContent(buf), CopyString);
    xmlBufferFree(buf);
    xmlFreeNode(copy);

    return_value = vm_call_user_func(type->map->to_zval,
                                          make_packed_array(data));
  }
  return return_value;
}

/* TODO: get rid of "bogus".. ither by passing in the already created xmlnode
   or passing in the node name */
/* String encode/decode */
static Variant to_zval_string(encodeTypePtr type, xmlNodePtr data) {
  USE_SOAP_GLOBAL;
  Variant ret;
  FIND_XML_NULL(data, ret);
  if (data && data->children) {
    if (data->children->type == XML_TEXT_NODE &&
        data->children->next == NULL) {
      if (SOAP_GLOBAL(encoding) != NULL) {
        xmlBufferPtr in =
          xmlBufferCreateStatic(data->children->content,
                                xmlStrlen(data->children->content));
        xmlBufferPtr out = xmlBufferCreate();
        int n = xmlCharEncOutFunc(SOAP_GLOBAL(encoding), out, in);
        if (n >= 0) {
          ret = String((char*)xmlBufferContent(out), CopyString);
        } else {
          ret = String((char*)data->children->content, CopyString);
        }
        xmlBufferFree(out);
        xmlBufferFree(in);
      } else {
        ret = String((char*)data->children->content, CopyString);
      }
    } else if (data->children->type == XML_CDATA_SECTION_NODE &&
               data->children->next == NULL) {
      ret = String((char*)data->children->content, CopyString);
    } else {
      throw SoapException("Encoding: Violation of encoding rules");
    }
  } else {
    ret = String("");
  }
  return ret;
}

static Variant to_zval_stringr(encodeTypePtr type, xmlNodePtr data) {
  USE_SOAP_GLOBAL;
  Variant ret;
  FIND_XML_NULL(data, ret);
  if (data && data->children) {
    if (data->children->type == XML_TEXT_NODE &&
        data->children->next == NULL) {
      whiteSpace_replace(data->children->content);
      if (SOAP_GLOBAL(encoding) != NULL) {
        xmlBufferPtr in =
          xmlBufferCreateStatic(data->children->content,
                                xmlStrlen(data->children->content));
        xmlBufferPtr out = xmlBufferCreate();
        int n = xmlCharEncOutFunc(SOAP_GLOBAL(encoding), out, in);
        if (n >= 0) {
          ret = String((char*)xmlBufferContent(out), CopyString);
        } else {
          ret = String((char*)data->children->content, CopyString);
        }
        xmlBufferFree(out);
        xmlBufferFree(in);
      } else {
        ret = String((char*)data->children->content, CopyString);
      }
    } else if (data->children->type == XML_CDATA_SECTION_NODE &&
               data->children->next == NULL) {
      ret = String((char*)data->children->content, CopyString);
    } else {
      throw SoapException("Encoding: Violation of encoding rules");
    }
  } else {
    ret = String("");
  }
  return ret;
}

static Variant to_zval_stringc(encodeTypePtr type, xmlNodePtr data) {
  USE_SOAP_GLOBAL;
  Variant ret;
  FIND_XML_NULL(data, ret);
  if (data && data->children) {
    if (data->children->type == XML_TEXT_NODE &&
        data->children->next == NULL) {
      whiteSpace_collapse(data->children->content);
      if (SOAP_GLOBAL(encoding) != NULL) {
        xmlBufferPtr in =
          xmlBufferCreateStatic(data->children->content,
                                xmlStrlen(data->children->content));
        xmlBufferPtr out = xmlBufferCreate();
        int n = xmlCharEncOutFunc(SOAP_GLOBAL(encoding), out, in);
        if (n >= 0) {
          ret = String((char*)xmlBufferContent(out), CopyString);
        } else {
          ret = String((char*)data->children->content, CopyString);
        }
        xmlBufferFree(out);
        xmlBufferFree(in);
      } else {
        ret = String((char*)data->children->content, CopyString);
      }
    } else if (data->children->type == XML_CDATA_SECTION_NODE &&
               data->children->next == NULL) {
      ret = String((char*)data->children->content, CopyString);
    } else {
      throw SoapException("Encoding: Violation of encoding rules");
    }
  } else {
    ret = String("");
  }
  return ret;
}

static Variant to_zval_base64(encodeTypePtr type, xmlNodePtr data) {
  Variant ret;
  FIND_XML_NULL(data, ret);
  if (data && data->children) {
    if (data->children->type == XML_TEXT_NODE &&
        data->children->next == NULL) {
      whiteSpace_collapse(data->children->content);
      String str =
        StringUtil::Base64Decode(String((const char*)data->children->content));
      if (str.isNull()) {
        throw SoapException("Encoding: Violation of encoding rules");
      }
      ret = str;
    } else if (data->children->type == XML_CDATA_SECTION_NODE &&
               data->children->next == NULL) {
      String str =
        StringUtil::Base64Decode(String((const char*)data->children->content));
      if (str.isNull()) {
        throw SoapException("Encoding: Violation of encoding rules");
      }
      ret = str;
    } else {
      throw SoapException("Encoding: Violation of encoding rules");
    }
  } else {
    ret = String("");
  }
  return ret;
}

static Variant to_zval_hexbin(encodeTypePtr type, xmlNodePtr data) {
  Variant ret;
  FIND_XML_NULL(data, ret);
  if (data && data->children) {
    if (data->children->type == XML_TEXT_NODE &&
        data->children->next == NULL) {
      whiteSpace_collapse(data->children->content);
    } else if (data->children->type != XML_CDATA_SECTION_NODE ||
               data->children->next != NULL) {
      throw SoapException("Encoding: Violation of encoding rules");
    }
    String str =
      f_hex2bin(String((const char*)data->children->content));
    if (str.isNull()) {
      throw SoapException("Encoding: Violation of encoding rules");
    }
    ret = str;
  } else {
    ret = String("");
  }
  return ret;
}

static xmlNodePtr to_xml_string(encodeTypePtr type, CVarRef data, int style,
                                xmlNodePtr parent) {
  USE_SOAP_GLOBAL;
  xmlNodePtr ret = xmlNewNode(NULL, BAD_CAST("BOGUS"));
  xmlAddChild(parent, ret);
  FIND_ZVAL_NULL(data, ret, style);

  String str = data.toString();
  if (SOAP_GLOBAL(encoding) != NULL) {
    xmlBufferPtr in  = xmlBufferCreateStatic((void*)str.data(), str.size());
    xmlBufferPtr out = xmlBufferCreate();
    int n = xmlCharEncInFunc(SOAP_GLOBAL(encoding), out, in);
    if (n >= 0) {
      str = String((char*)xmlBufferContent(out), n, CopyString);
    }
    xmlBufferFree(out);
    xmlBufferFree(in);
  }

  if (!php_libxml_xmlCheckUTF8(BAD_CAST(str.data()))) {
    char *err = (char*)malloc(str.size() + 8);
    char c;
    memcpy(err, str.data(), str.size() + 1);
    int i = 0;
    while ((c = err[i++])) {
      if ((c & 0x80) == 0) {
      } else if ((c & 0xe0) == 0xc0) {
        if ((err[i] & 0xc0) != 0x80) {
          break;
        }
        i++;
      } else if ((c & 0xf0) == 0xe0) {
        if ((err[i] & 0xc0) != 0x80 || (err[i+1] & 0xc0) != 0x80) {
          break;
        }
        i += 2;
      } else if ((c & 0xf8) == 0xf0) {
        if ((err[i] & 0xc0) != 0x80 ||
            (err[i+1] & 0xc0) != 0x80 || (err[i+2] & 0xc0) != 0x80) {
          break;
        }
        i += 3;
      } else {
        break;
      }
    }
    if (c) {
      err[i-1] = '\\';
      err[i++] = 'x';
      err[i++] = ((unsigned char)c >> 4) +
        ((((unsigned char)c >> 4) > 9) ? ('a' - 10) : '0');
      err[i++] = (c & 15) + (((c & 15) > 9) ? ('a' - 10) : '0');
      err[i++] = '.';
      err[i++] = '.';
      err[i++] = '.';
      err[i++] = 0;
    }
    string serr = err;
    free(err);
    throw SoapException("Encoding: string '%s' is not a valid utf-8 string",
                        serr.c_str());
  }

  xmlNodePtr text = xmlNewTextLen(BAD_CAST(str.data()), str.size());
  xmlAddChild(ret, text);

  if (style == SOAP_ENCODED) {
    set_ns_and_type(ret, type);
  }
  return ret;
}

static xmlNodePtr to_xml_base64(encodeTypePtr type, CVarRef data, int style,
                                xmlNodePtr parent) {
  xmlNodePtr ret = xmlNewNode(NULL, BAD_CAST("BOGUS"));
  xmlAddChild(parent, ret);
  FIND_ZVAL_NULL(data, ret, style);

  String str = StringUtil::Base64Encode(data.toString());
  xmlAddChild(ret, xmlNewTextLen(BAD_CAST(str.data()), str.size()));

  if (style == SOAP_ENCODED) {
    set_ns_and_type(ret, type);
  }
  return ret;
}

static xmlNodePtr to_xml_hexbin(encodeTypePtr type, CVarRef data, int style,
                                xmlNodePtr parent) {
  xmlNodePtr ret = xmlNewNode(NULL, BAD_CAST("BOGUS"));
  xmlAddChild(parent, ret);
  FIND_ZVAL_NULL(data, ret, style);

  String str = f_bin2hex(data.toString());
  xmlAddChild(ret, xmlNewTextLen(BAD_CAST(str.data()), str.size()));

  if (style == SOAP_ENCODED) {
    set_ns_and_type(ret, type);
  }
  return ret;
}

static Variant to_zval_double(encodeTypePtr type, xmlNodePtr data) {
  Variant ret;
  FIND_XML_NULL(data, ret);
  if (data && data->children) {
    if (data->children->type == XML_TEXT_NODE &&
        data->children->next == NULL) {
      int64_t lval; double dval;
      whiteSpace_collapse(data->children->content);
      String content((char*)data->children->content, CopyString);
      switch (is_numeric_string((const char *)data->children->content,
                                data->children->content ?
                                strlen((char*)data->children->content) : 0,
                                &lval, &dval, 0)) {
      case KindOfInt64:  ret = lval; break;
      case KindOfDouble: ret = dval; break;
      default:
        if (data->children->content) {
          if (strcasecmp((const char *)data->children->content, "NaN") == 0) {
            ret = atof("nan"); break;
          } else if (strcasecmp((const char *)data->children->content, "INF")
                     == 0) {
            ret = atof("inf"); break;
          } else if (strcasecmp((const char *)data->children->content, "-INF")
                     == 0) {
            ret = -atof("inf"); break;
          }
        }
        throw SoapException("Encoding: Violation of encoding rules");
      }
    } else {
      throw SoapException("Encoding: Violation of encoding rules");
    }
  }
  return ret;
}

static Variant to_zval_long(encodeTypePtr type, xmlNodePtr data) {
  Variant ret;
  FIND_XML_NULL(data, ret);
  if (data && data->children) {
    if (data->children->type == XML_TEXT_NODE &&
        data->children->next == NULL) {
      int64_t lval; double dval;
      whiteSpace_collapse(data->children->content);
      switch (is_numeric_string((const char *)data->children->content,
                                data->children->content ?
                                strlen((char*)data->children->content) : 0,
                                &lval, &dval, 0)) {
      case KindOfInt64:  ret = (int64_t)lval; break;
      case KindOfDouble: ret = dval; break;
      default:
        throw SoapException("Encoding: Violation of encoding rules");
      }
    } else {
      throw SoapException("Encoding: Violation of encoding rules");
    }
  }
  return ret;
}

static xmlNodePtr to_xml_long(encodeTypePtr type, CVarRef data, int style,
                              xmlNodePtr parent) {
  xmlNodePtr ret = xmlNewNode(NULL, BAD_CAST("BOGUS"));
  xmlAddChild(parent, ret);
  FIND_ZVAL_NULL(data, ret, style);

  if (data.isDouble()) {
    char s[256];
    snprintf(s, sizeof(s), "%0.0F", floor(data.toDouble()));
    xmlNodeSetContent(ret, BAD_CAST(s));
  } else {
    Variant d = data.toInt64();
    String sd = d.toString();
    xmlNodeSetContentLen(ret, BAD_CAST(sd.data()), sd.size());
  }

  if (style == SOAP_ENCODED) {
    set_ns_and_type(ret, type);
  }
  return ret;
}

static xmlNodePtr to_xml_double(encodeTypePtr type, CVarRef data, int style,
                                xmlNodePtr parent) {
  xmlNodePtr ret = xmlNewNode(NULL, BAD_CAST("BOGUS"));
  xmlAddChild(parent, ret);
  FIND_ZVAL_NULL(data, ret, style);

  Variant d = data.toDouble();
  String sd = d.toString();
  xmlNodeSetContentLen(ret, BAD_CAST(sd.data()), sd.size());

  if (style == SOAP_ENCODED) {
    set_ns_and_type(ret, type);
  }
  return ret;
}

static Variant to_zval_bool(encodeTypePtr type, xmlNodePtr data) {
  Variant ret;
  FIND_XML_NULL(data, ret);
  if (data && data->children) {
    if (data->children->type == XML_TEXT_NODE &&
        data->children->next == NULL) {
      whiteSpace_collapse(data->children->content);
      const char *content = (const char *)data->children->content;
      if (strcasecmp((char*)content, "true") == 0 ||
          strcasecmp((char*)content, "t") == 0 ||
          strcmp((char*)content, "1") == 0) {
        ret = true;
      } else if (strcasecmp((char*)content, "false") == 0 ||
                 strcasecmp((char*)content, "f") == 0 ||
                 strcmp((char*)content, "0") == 0) {
        ret = false;
      } else {
        ret = String((char*)content, CopyString).toBoolean();
      }
    } else {
      throw SoapException("Encoding: Violation of encoding rules");
    }
  }
  return ret;
}

static xmlNodePtr to_xml_bool(encodeTypePtr type, CVarRef data, int style,
                              xmlNodePtr parent) {
  xmlNodePtr ret = xmlNewNode(NULL, BAD_CAST("BOGUS"));
  xmlAddChild(parent, ret);
  FIND_ZVAL_NULL(data, ret, style);

  if (data.toBoolean()) {
    xmlNodeSetContent(ret, BAD_CAST("true"));
  } else {
    xmlNodeSetContent(ret, BAD_CAST("false"));
  }

  if (style == SOAP_ENCODED) {
    set_ns_and_type(ret, type);
  }
  return ret;
}

/* Null encode/decode */
static Variant to_zval_null(encodeTypePtr type, xmlNodePtr data) {
  return uninit_null();
}

static xmlNodePtr to_xml_null(encodeTypePtr type, CVarRef data, int style,
                              xmlNodePtr parent) {
  xmlNodePtr ret = xmlNewNode(NULL, BAD_CAST("BOGUS"));
  xmlAddChild(parent, ret);
  if (style == SOAP_ENCODED) {
    set_xsi_nil(ret);
  }
  return ret;
}

static bool get_zval_property(Variant &object, const char* name,
                              Variant *ret = NULL) {
  String sname(name);
  if (object.isObject()) {
    Object obj = object.toObject();
    if (Variant* t = obj->o_realProp(sname, ObjectData::RealPropUnchecked)) {
      if (t->isInitialized()) {
        if (ret) ret->assignRef(*t);
        return true;
      }
    }
    return false;
  }
  if (object.isArray()) {
    Array arr = object.toArray();
    if (!arr.exists(sname)) {
      return false;
    }
    if (ret) ret->assignRef(object.lvalAt(sname));
    return true;
  }
  return false;
}

static void model_to_zval_any(Variant &ret, xmlNodePtr node) {
  const char* name = nullptr;
  Variant any;
  while (node != NULL) {
    if (!get_zval_property(ret, (const char *)node->name)) {
      Variant val = master_to_zval(get_conversion(XSD_ANYXML), node);

      if (!any.isNull() && !any.isArray()) {
        Array arr = Array::Create();
        if (name) {
          arr.set(String(name, CopyString), any);
        } else {
          arr.append(any);
        }
        any = arr;
      }

      if (val.isString() && val.toString().charAt(0) == '<') {
        name = NULL;
        while (node->next != NULL) {
          Variant val2 = master_to_zval(get_conversion(XSD_ANYXML),
                                        node->next);
          if (!val2.isString()) {
            break;
          }
          concat_assign(val, val2.toString());
          node = node->next;
        }
      } else {
        name = (const char*)node->name;
      }

      if (any.isNull()) {
        if (name) {
          Array arr = Array::Create();
          arr.set(String(name, CopyString), val);
          any = arr;
          name = NULL;
        } else {
          any = val;
        }
      } else {
        /* Add array element */
        if (name) {
          String name_str(name);
          if (any.toArray().exists(name_str)) {
            Variant &el = any.lvalAt(name_str);
            if (!el.isArray()) {
              /* Convert into array */
              Array arr = Array::Create();
              arr.append(el);
              el = arr;
            }
            el.append(val);
          } else {
            any.set(name_str, val);
          }
        } else {
          any.append(val);
        }
        name = NULL;
      }
    }
    node = node->next;
  }
  if (any.toBoolean()) {
    ret.toObject()->o_set(name ? String(name, CopyString) : "any", any);
  }
}

static void model_to_zval_object(Variant &ret, sdlContentModelPtr model,
                                 xmlNodePtr data, sdl *sdl) {
  USE_SOAP_GLOBAL;
  switch (model->kind) {
  case XSD_CONTENT_ELEMENT:
    if (!model->u_element->name.empty()) {
      xmlNodePtr node = get_node(data->children,
                                 (char*)model->u_element->name.c_str());
      if (node) {
        Variant val;
        xmlNodePtr r_node = check_and_resolve_href(node);
        if (r_node && r_node->children && r_node->children->content) {
          if (!model->u_element->fixed.empty() &&
              model->u_element->fixed != (char*)r_node->children->content) {
            throw SoapException("Encoding: Element '%s' has fixed value '%s' "
                                "(value '%s' is not allowed)",
                                model->u_element->name.c_str(),
                                model->u_element->fixed.c_str(),
                                r_node->children->content);
          }
          val = master_to_zval(model->u_element->encode, r_node);
        } else if (!model->u_element->fixed.empty()) {
          xmlNodePtr dummy = xmlNewNode(NULL, BAD_CAST("BOGUS"));
          xmlNodeSetContent(dummy, BAD_CAST(model->u_element->fixed.c_str()));
          val = master_to_zval(model->u_element->encode, dummy);
          xmlFreeNode(dummy);
        } else if (!model->u_element->def.empty() &&
                   !model->u_element->nillable) {
          xmlNodePtr dummy = xmlNewNode(NULL, BAD_CAST("BOGUS"));
          xmlNodeSetContent(dummy,
                            BAD_CAST(model->u_element->def.c_str()));
          val = master_to_zval(model->u_element->encode, dummy);
          xmlFreeNode(dummy);
        } else {
          val = master_to_zval(model->u_element->encode, r_node);
        }
        if ((node = get_node(node->next,
                             (char*)model->u_element->name.c_str())) != NULL) {
          Array array = Array::Create();
          array.append(val);
          do {
            if (node && node->children && node->children->content) {
              if (!model->u_element->fixed.empty() &&
                  model->u_element->fixed != (char*)node->children->content) {
                throw SoapException("Encoding: Element '%s' has fixed value "
                                    "'%s' (value '%s' is not allowed)",
                                    model->u_element->name.c_str(),
                                    model->u_element->fixed.c_str(),
                                    node->children->content);
              }
              val = master_to_zval(model->u_element->encode, node);
            } else if (!model->u_element->fixed.empty()) {
              xmlNodePtr dummy = xmlNewNode(NULL, BAD_CAST("BOGUS"));
              xmlNodeSetContent(dummy,
                                BAD_CAST(model->u_element->fixed.c_str()));
              val = master_to_zval(model->u_element->encode, dummy);
              xmlFreeNode(dummy);
            } else if (!model->u_element->def.empty() &&
                       !model->u_element->nillable) {
              xmlNodePtr dummy = xmlNewNode(NULL, BAD_CAST("BOGUS"));
              xmlNodeSetContent(dummy,
                                BAD_CAST(model->u_element->def.c_str()));
              val = master_to_zval(model->u_element->encode, dummy);
              xmlFreeNode(dummy);
            } else {
              val = master_to_zval(model->u_element->encode, node);
            }
            array.append(val);
          } while ((node = get_node
                    (node->next,
                     (char*)model->u_element->name.c_str())) != NULL);
          val = array;
        } else if ((!val.isNull() || !model->u_element->nillable) &&
                   (SOAP_GLOBAL(features) & SOAP_SINGLE_ELEMENT_ARRAYS) &&
                   (model->max_occurs == -1 || model->max_occurs > 1)) {
          Array array = Array::Create();
          array.append(val);
          val = array;
        }
        ret.toObject()->o_set(String(model->u_element->name), val);
      }
    }
    break;
  case XSD_CONTENT_ALL:
  case XSD_CONTENT_SEQUENCE:
  case XSD_CONTENT_CHOICE: {
    sdlContentModelPtr any;
    for (unsigned int i = 0; i < model->u_content.size(); i++) {
      sdlContentModelPtr content = model->u_content[i];
      if (content->kind == XSD_CONTENT_ANY) {
        any = content;
      } else {
        model_to_zval_object(ret, content, data, sdl);
      }
    }
    if (any) {
      model_to_zval_any(ret, data->children);
    }
    break;
  }
  case XSD_CONTENT_GROUP:
    model_to_zval_object(ret, model->u_group->model, data, sdl);
    break;
  default:
    break;
  }
}

/* Struct encode/decode */
static Variant to_zval_object_ex(encodeTypePtr type, xmlNodePtr data,
                                 const char *pce) {
  USE_SOAP_GLOBAL;
  const char *ce = "stdClass";
  String clsname;
  if (pce) {
    ce = pce;
  } else if (!SOAP_GLOBAL(classmap).empty() && !type->type_str.empty()) {
    String type_str(type->type_str);
    if (SOAP_GLOBAL(classmap).exists(type_str)) {
      clsname = SOAP_GLOBAL(classmap)[type_str].toString();
      ce = clsname.data();
    }
  }

  Variant ret;
  bool redo_any = false;
  sdlType *sdlType = type->sdl_type;
  sdl *sdl = SOAP_GLOBAL(sdl);
  if (sdlType) {
    if (sdlType->kind == XSD_TYPEKIND_RESTRICTION &&
        sdlType->encode && type != &sdlType->encode->details) {
      encodePtr enc = sdlType->encode;
      while (enc && enc->details.sdl_type &&
             enc->details.sdl_type->kind != XSD_TYPEKIND_SIMPLE &&
             enc->details.sdl_type->kind != XSD_TYPEKIND_LIST &&
             enc->details.sdl_type->kind != XSD_TYPEKIND_UNION) {
        enc = enc->details.sdl_type->encode;
      }
      if (enc) {
        if (soap_check_xml_ref(ret, data)) {
          return ret;
        }
        ret = create_object(ce, Array());
        ret.toObject()->o_set("_", master_to_zval_int(enc, data));
      } else {
        FIND_XML_NULL(data, ret);
        if (soap_check_xml_ref(ret, data)) {
          return ret;
        }
        ret = create_object(ce, Array());
      }
    } else if (sdlType->kind == XSD_TYPEKIND_EXTENSION &&
               sdlType->encode &&
               type != &sdlType->encode->details) {
      encodeType &details = sdlType->encode->details;
      if (details.sdl_type &&
          details.sdl_type->kind != XSD_TYPEKIND_SIMPLE &&
          details.sdl_type->kind != XSD_TYPEKIND_LIST &&
          details.sdl_type->kind != XSD_TYPEKIND_UNION) {

        if (strcasecmp(ce, "stdClass") != 0 &&
            sdlType->encode->to_zval == sdl_guess_convert_zval &&
            details.sdl_type != NULL &&
            (details.sdl_type->kind == XSD_TYPEKIND_COMPLEX ||
             details.sdl_type->kind == XSD_TYPEKIND_RESTRICTION ||
             details.sdl_type->kind == XSD_TYPEKIND_EXTENSION) &&
            (details.sdl_type->encode == NULL ||
             (details.sdl_type->encode->details.type != KindOfArray &&
              details.sdl_type->encode->details.type != SOAP_ENC_ARRAY))) {
          ret = to_zval_object_ex(&sdlType->encode->details, data, ce);
        } else {
          ret = master_to_zval_int(sdlType->encode, data);
        }
        FIND_XML_NULL(data, ret);
        if (soap_check_xml_ref(ret, data)) {
          return ret;
        }
        redo_any = get_zval_property(ret, "any");
        ret.toObject()->o_set("any", uninit_null());
      } else {
        if (soap_check_xml_ref(ret, data)) {
          return ret;
        }
        ret = create_object(ce, Array());
        ret.toObject()->o_set("_", master_to_zval_int(sdlType->encode, data));
      }
    } else {
      FIND_XML_NULL(data, ret);
      if (soap_check_xml_ref(ret, data)) {
        return ret;
      }
      ret = create_object(ce, Array());
    }
    if (sdlType->model) {
      if (redo_any) {
        ret.toObject()->o_set("any", uninit_null());
      }
      model_to_zval_object(ret, sdlType->model, data, sdl);
      if (redo_any) {
        model_to_zval_any(ret, data->children);
      }
    }
    if (!sdlType->attributes.empty()) {
      for (sdlAttributeMap::const_iterator iter = sdlType->attributes.begin();
           iter != sdlType->attributes.end(); ++iter) {
        sdlAttributePtr attr = iter->second;
        if (!attr->name.empty()) {
          xmlAttrPtr val = get_attribute(data->properties,
                                         (char*)attr->name.c_str());
          const char *str_val = NULL;
          if (val && val->children && val->children->content) {
            str_val = (char*)val->children->content;
            if (!attr->fixed.empty() && attr->fixed != str_val) {
              throw SoapException("Encoding: Attribute '%s' has fixed value"
                                  " '%s' (value '%s' is not allowed)",
                                  attr->name.c_str(), attr->fixed.c_str(),
                                  str_val);
            }
          } else if (!attr->fixed.empty()) {
            str_val = attr->fixed.c_str();
          } else if (!attr->def.empty()) {
            str_val = attr->def.c_str();
          }
          if (str_val) {
            xmlNodePtr dummy = xmlNewNode(NULL, BAD_CAST("BOGUS"));
            xmlNodePtr text = xmlNewText(BAD_CAST(str_val));
            xmlAddChild(dummy, text);
            Variant data = master_to_zval(attr->encode, dummy);
            xmlFreeNode(dummy);
            ret.toObject()->o_set(String(attr->name), data);
          }
        }
      }
    }
  } else {
    FIND_XML_NULL(data, ret);
    if (soap_check_xml_ref(ret, data)) {
      return ret;
    }

    ret = create_object(ce, Array());
    xmlNodePtr trav = data->children;
    while (trav != NULL) {
      if (trav->type == XML_ELEMENT_NODE) {
        Variant tmpVal = master_to_zval(encodePtr(), trav);
        Variant prop;
        if (!get_zval_property(ret, (char*)trav->name, &prop)) {
          if (!trav->next || !get_node(trav->next, (char*)trav->name)) {
            ret.toObject()->o_set(
              String((char*)trav->name, CopyString), tmpVal);
          } else {
            Array arr = Array::Create();
            arr.append(tmpVal);
            ret.toObject()->o_set(String((char*)trav->name, CopyString), arr);
          }
        } else {
          /* Property already exist - make array */
          if (!prop.isArray()) {
            /* Convert into array */
            Array arr = Array::Create();
            arr.append(prop);
            ret.toObject()->o_set(String((char*)trav->name, CopyString), arr);
            prop = arr;
          }
          /* Add array element */
          prop.append(tmpVal);
        }
      }
      trav = trav->next;
    }
  }
  return ret;
}

static Variant to_zval_object(encodeTypePtr type, xmlNodePtr data) {
  return to_zval_object_ex(type, data, NULL);
}

static int model_to_xml_object(xmlNodePtr node, sdlContentModelPtr model,
                               Variant &object, int style, int strict) {
  switch (model->kind) {
  case XSD_CONTENT_ELEMENT: {
    xmlNodePtr property;
    encodePtr enc;

    Variant data;
    if (get_zval_property(object, model->u_element->name.c_str(), &data) &&
        data.isNull() && !model->u_element->nillable &&
        model->min_occurs > 0 && !strict) {
      return 0;
    }
    if (!data.isNull()) {
      enc = model->u_element->encode;
      if ((model->max_occurs == -1 || model->max_occurs > 1) &&
          data.isArray() && data.toArray()->isVectorData()) {
        for (ArrayIter iter(data.toArray()); iter; ++iter) {
          Variant val = iter.second();
          if (val.isNull() && model->u_element->nillable) {
            property = xmlNewNode(NULL, BAD_CAST("BOGUS"));
            xmlAddChild(node, property);
            set_xsi_nil(property);
          } else {
            property = master_to_xml(enc, val, style, node);
            if (property->children && property->children->content &&
                !model->u_element->fixed.empty() &&
                model->u_element->fixed != (char*)property->children->content){
              throw SoapException("Encoding: Element '%s' has fixed value "
                                  "'%s' (value '%s' is not allowed)",
                                  model->u_element->name.c_str(),
                                  model->u_element->fixed.c_str(),
                                  property->children->content);
            }
          }
          xmlNodeSetName(property, BAD_CAST(model->u_element->name.c_str()));
          if (style == SOAP_LITERAL &&
              !model->u_element->namens.empty() &&
              model->u_element->form == XSD_FORM_QUALIFIED) {
            xmlNsPtr nsp = encode_add_ns(property,
                                         model->u_element->namens.c_str());
            xmlSetNs(property, nsp);
          }
        }
      } else {
        if (data.isNull() && model->u_element->nillable) {
          property = xmlNewNode(NULL, BAD_CAST("BOGUS"));
          xmlAddChild(node, property);
          set_xsi_nil(property);
        } else if (data.isNull() && model->min_occurs == 0) {
          return 1;
        } else {
          property = master_to_xml(enc, data, style, node);
          if (property->children && property->children->content &&
              !model->u_element->fixed.empty() &&
              model->u_element->fixed != (char*)property->children->content) {
            throw SoapException("Encoding: Element '%s' has fixed value '%s' "
                                "(value '%s' is not allowed)",
                                model->u_element->name.c_str(),
                                model->u_element->fixed.c_str(),
                                property->children->content);
          }
        }
        xmlNodeSetName(property, BAD_CAST(model->u_element->name.c_str()));
        if (style == SOAP_LITERAL &&
            !model->u_element->namens.empty() &&
            model->u_element->form == XSD_FORM_QUALIFIED) {
          xmlNsPtr nsp = encode_add_ns(property,
                                       model->u_element->namens.c_str());
          xmlSetNs(property, nsp);
        }
      }
      return 1;
    } else if (strict && model->u_element->nillable && model->min_occurs > 0) {
      property = xmlNewNode(NULL, BAD_CAST(model->u_element->name.c_str()));
      xmlAddChild(node, property);
      set_xsi_nil(property);
      if (style == SOAP_LITERAL &&
          model->u_element->form == XSD_FORM_QUALIFIED) {
        xmlNsPtr nsp = encode_add_ns(property,
                                     model->u_element->namens.c_str());
        xmlSetNs(property, nsp);
      }
      return 1;
    } else if (model->min_occurs == 0) {
      return 2;
    } else {
      if (strict) {
        throw SoapException("Encoding: object hasn't '%s' property",
                            model->u_element->name.c_str());
      }
      return 0;
    }
    break;
  }
  case XSD_CONTENT_ANY: {
    encodePtr enc;

    Variant data;
    if (get_zval_property(object, "any", &data)) {
      enc = get_conversion(XSD_ANYXML);
      if ((model->max_occurs == -1 || model->max_occurs > 1) &&
          data.isArray() && data.toArray()->isVectorData()) {
        for (ArrayIter iter(data.toArray()); iter; ++iter) {
          master_to_xml(enc, iter.second(), style, node);
        }
      } else {
        master_to_xml(enc, data, style, node);
      }
      return 1;
    } else if (model->min_occurs == 0) {
      return 2;
    } else {
      if (strict) {
        throw SoapException( "Encoding: object hasn't 'any' property");
      }
      return 0;
    }
    break;
  }
  case XSD_CONTENT_SEQUENCE:
  case XSD_CONTENT_ALL:
    for (unsigned int i = 0; i < model->u_content.size(); i++) {
      sdlContentModelPtr tmp = model->u_content[i];
      if (!model_to_xml_object(node,tmp,object,style,tmp->min_occurs > 0) &&
          tmp->min_occurs > 0) {
        return 0;
      }
    }
    return 1;
  case XSD_CONTENT_CHOICE: {
    int ret = 0;
    for (unsigned int i = 0; i < model->u_content.size(); i++) {
      sdlContentModelPtr tmp = model->u_content[i];
      int tmp_ret = model_to_xml_object(node, tmp, object, style, 0);
      if (tmp_ret == 1) {
        return 1;
      } else if (tmp_ret != 0) {
        ret = 1;
      }
    }
    return ret;
  }
  case XSD_CONTENT_GROUP:
    return model_to_xml_object(node, model->u_group->model, object, style,
                               model->min_occurs > 0);
  default:
    break;
  }
  return 1;
}

static sdlTypePtr model_array_element(sdlContentModelPtr model) {
  switch (model->kind) {
  case XSD_CONTENT_ELEMENT: {
    if (model->max_occurs == -1 || model->max_occurs > 1) {
      return sdlTypePtr(model->u_element, null_deleter());
    } else {
      return sdlTypePtr();
    }
  }
  case XSD_CONTENT_SEQUENCE:
  case XSD_CONTENT_ALL:
  case XSD_CONTENT_CHOICE: {
    if (model->u_content.size() != 1) {
      return sdlTypePtr();
    }
    return model_array_element(model->u_content[0]);
  }
  case XSD_CONTENT_GROUP:
    return model_array_element(model->u_group->model);
  default:
    break;
  }
  return sdlTypePtr();
}

static xmlNodePtr to_xml_object(encodeTypePtr type, CVarRef data_, int style,
                                xmlNodePtr parent) {
  xmlNodePtr xmlParam;
  sdlType *sdlType = type->sdl_type;
  Variant data = data_;

  if (data.isNull()) {
    xmlParam = xmlNewNode(NULL, BAD_CAST("BOGUS"));
    xmlAddChild(parent, xmlParam);
    if (style == SOAP_ENCODED) {
      set_xsi_nil(xmlParam);
      set_ns_and_type(xmlParam, type);
    }
    return xmlParam;
  }

  Array prop;
  if (data.isObject() || data.isArray()) {
    prop = data.toArray();
  }

  if (sdlType) {
    if (sdlType->kind == XSD_TYPEKIND_RESTRICTION &&
        sdlType->encode && type != &sdlType->encode->details) {
      encodePtr enc = sdlType->encode;
      while (enc && enc->details.sdl_type &&
             enc->details.sdl_type->kind != XSD_TYPEKIND_SIMPLE &&
             enc->details.sdl_type->kind != XSD_TYPEKIND_LIST &&
             enc->details.sdl_type->kind != XSD_TYPEKIND_UNION) {
        enc = enc->details.sdl_type->encode;
      }
      if (enc) {
        Variant tmp;
        if (get_zval_property(data, "_", &tmp)) {
          xmlParam = master_to_xml(enc, tmp, style, parent);
        } else if (prop.isNull()) {
          xmlParam = master_to_xml(enc, data, style, parent);
        } else {
          xmlParam = xmlNewNode(NULL, BAD_CAST("BOGUS"));
          xmlAddChild(parent, xmlParam);
        }
      } else {
        xmlParam = xmlNewNode(NULL, BAD_CAST("BOGUS"));
        xmlAddChild(parent, xmlParam);
      }
    } else if (sdlType->kind == XSD_TYPEKIND_EXTENSION &&
               sdlType->encode && type != &sdlType->encode->details) {
      if (sdlType->encode->details.sdl_type &&
          sdlType->encode->details.sdl_type->kind != XSD_TYPEKIND_SIMPLE &&
          sdlType->encode->details.sdl_type->kind != XSD_TYPEKIND_LIST &&
          sdlType->encode->details.sdl_type->kind != XSD_TYPEKIND_UNION) {
        xmlParam = master_to_xml(sdlType->encode, data, style, parent);
      } else {
        Variant tmp;
        if (get_zval_property(data, "_", &tmp)) {
          xmlParam = master_to_xml(sdlType->encode, tmp, style, parent);
        } else if (prop.isNull()) {
          xmlParam = master_to_xml(sdlType->encode, data, style, parent);
        } else {
          xmlParam = xmlNewNode(NULL, BAD_CAST("BOGUS"));
          xmlAddChild(parent, xmlParam);
        }
      }
    } else {
      xmlParam = xmlNewNode(NULL, BAD_CAST("BOGUS"));
      xmlAddChild(parent, xmlParam);
    }

    if (soap_check_zval_ref(data, xmlParam)) {
      return xmlParam;
    }
    if (!prop.isNull()) {
      sdlTypePtr array_el;

      if (data.isArray() && data.toArray()->isVectorData() &&
          sdlType->attributes.empty() && sdlType->model != NULL &&
          (array_el = model_array_element(sdlType->model)) != NULL) {
        for (ArrayIter iter(prop); iter; ++iter) {
          Variant val = iter.second();
          xmlNodePtr property;
          if (val.isNull() && array_el->nillable) {
            property = xmlNewNode(NULL, BAD_CAST("BOGUS"));
            xmlAddChild(xmlParam, property);
            set_xsi_nil(property);
          } else {
            property = master_to_xml(array_el->encode, val, style, xmlParam);
          }
          xmlNodeSetName(property, BAD_CAST(array_el->name.c_str()));
          if (style == SOAP_LITERAL && !array_el->namens.empty() &&
              array_el->form == XSD_FORM_QUALIFIED) {
            xmlNsPtr nsp = encode_add_ns(property, array_el->namens.c_str());
            xmlSetNs(property, nsp);
          }
        }
      } else if (sdlType->model) {
        model_to_xml_object(xmlParam, sdlType->model, data, style, 1);
      }
      if (!sdlType->attributes.empty()) {
        for (sdlAttributeMap::const_iterator iter =
               sdlType->attributes.begin(); iter != sdlType->attributes.end();
             ++iter) {
          sdlAttributePtr attr = iter->second;
          if (!attr->name.empty()) {
            Variant zattr;
            if (get_zval_property(data, attr->name.c_str(), &zattr)) {
              xmlNodePtr dummy = master_to_xml(attr->encode, zattr,
                                               SOAP_LITERAL, xmlParam);
              if (dummy->children && dummy->children->content) {
                if (!attr->fixed.empty() &&
                    attr->fixed != (char*)dummy->children->content) {
                  throw SoapException("Encoding: Attribute '%s' has fixed "
                                      "value '%s' (value '%s' is not allowed)",
                                      attr->name.c_str(), attr->fixed.c_str(),
                                      dummy->children->content);
                }
                /* we need to handle xml: namespace specially, since it is
                   an implicit schema. Otherwise, use form. */
                if (!strncmp(attr->namens.c_str(), XML_NAMESPACE,
                             sizeof(XML_NAMESPACE)) ||
                    attr->form == XSD_FORM_QUALIFIED) {
                  xmlNsPtr nsp = encode_add_ns(xmlParam, attr->namens.c_str());
                  xmlSetNsProp(xmlParam, nsp, BAD_CAST(attr->name.c_str()),
                               dummy->children->content);
                } else {
                  xmlSetProp(xmlParam, BAD_CAST(attr->name.c_str()),
                             dummy->children->content);
                }
              }
              xmlUnlinkNode(dummy);
              xmlFreeNode(dummy);
            }
          }
        }
      }
    }
    if (style == SOAP_ENCODED) {
      set_ns_and_type(xmlParam, type);
    }
  } else {
    xmlParam = xmlNewNode(NULL, BAD_CAST("BOGUS"));
    xmlAddChild(parent, xmlParam);

    if (soap_check_zval_ref(data, xmlParam)) {
      return xmlParam;
    }
    if (!prop.isNull()) {
      for (ArrayIter iter(prop); iter; ++iter) {
        Variant key = iter.first();
        Variant zprop = iter.second();
        xmlNodePtr property = master_to_xml(get_conversion(zprop.getType()),
                                            zprop, style, xmlParam);
        if (key.isString()) {
          String skey = key.toString();
          const char *prop_name;
          if (data.isObject()) {
            const char *class_name;
            zend_unmangle_property_name(skey.data(), skey.size(),
                                        &class_name, &prop_name);
          } else {
            prop_name = skey.data();
          }
          if (prop_name) {
            xmlNodeSetName(property, BAD_CAST(prop_name));
          }
        }
      }
    }
    if (style == SOAP_ENCODED) {
      set_ns_and_type(xmlParam, type);
    }
  }
  return xmlParam;
}

/* Array encode/decode */
static xmlNodePtr guess_array_map(encodeTypePtr type, CVarRef data, int style,
                                  xmlNodePtr parent) {
  encodePtr enc;

  if (data.isArray()) {
    if (!data.toArray()->isVectorData()) {
      enc = get_conversion(APACHE_MAP);
    } else {
      enc = get_conversion(SOAP_ENC_ARRAY);
    }
  }
  if (!enc) {
    enc = get_conversion(KindOfNull);
  }

  return master_to_xml(enc, data, style, parent);
}

static int calc_dimension_12(const char* str) {
  int i = 0, flag = 0;
  while (*str != '\0' && (*str < '0' || *str > '9') && (*str != '*')) {
    str++;
  }
  if (*str == '*') {
    i++;
    str++;
  }
  while (*str != '\0') {
    if (*str >= '0' && *str <= '9') {
      if (flag == 0) {
         i++;
         flag = 1;
       }
    } else if (*str == '*') {
      throw SoapException("Encoding: '*' may only be first arraySize value"
                          " in list");
    } else {
      flag = 0;
    }
    str++;
  }
  return i;
}

static int* get_position_12(int dimension, const char* str) {
  int *pos;
  int i = -1, flag = 0;

  pos = (int*)calloc(dimension, sizeof(int));
  while (*str != '\0' && (*str < '0' || *str > '9') && (*str != '*')) {
    str++;
  }
  if (*str == '*') {
    str++;
    i++;
  }
  while (*str != '\0') {
    if (*str >= '0' && *str <= '9') {
      if (flag == 0) {
        i++;
        flag = 1;
      }
      pos[i] = (pos[i]*10)+(*str-'0');
    } else if (*str == '*') {
      throw SoapException("Encoding: '*' may only be first arraySize value in list");
    } else {
      flag = 0;
    }
    str++;
  }
  return pos;
}

static int calc_dimension(const char* str) {
  int i = 1;
  while (*str != ']' && *str != '\0') {
    if (*str == ',') {
        i++;
    }
    str++;
  }
  return i;
}

static void get_position_ex(int dimension, const char* str, int** pos) {
  int i = 0;

  memset(*pos,0,sizeof(int)*dimension);
  while (*str != ']' && *str != '\0' && i < dimension) {
    if (*str >= '0' && *str <= '9') {
      (*pos)[i] = ((*pos)[i]*10)+(*str-'0');
    } else if (*str == ',') {
      i++;
    }
    str++;
  }
}

static int* get_position(int dimension, const char* str) {
  int *pos = (int*)malloc(sizeof(int) * dimension);
  get_position_ex(dimension, str, &pos);
  return pos;
}

static void add_xml_array_elements(xmlNodePtr xmlParam,
                                   sdlTypePtr type,
                                   encodePtr enc,
                                   xmlNsPtr ns,
                                   int dimension ,
                                   int* dims,
                                   CVarRef data,
                                   int style) {
  if (data.isArray()) {
    Array arr = data.toArray();
    ArrayIter iter(arr);
    for (int j=0; j<dims[0]; j++) {
      Variant zdata = iter.second(); ++iter;
      if (dimension == 1) {
        xmlNodePtr xparam;
        if (!zdata.isNull()) {
          if (enc == NULL) {
            xparam = master_to_xml(get_conversion(zdata.getType()), zdata,
                                   style, xmlParam);
          } else {
            xparam = master_to_xml(enc, zdata, style, xmlParam);
          }
        } else {
          xparam = xmlNewNode(NULL, BAD_CAST("BOGUS"));
          xmlAddChild(xmlParam, xparam);
        }

        if (type) {
          xmlNodeSetName(xparam, BAD_CAST(type->name.c_str()));
        } else if (style == SOAP_LITERAL && enc &&
                   !enc->details.type_str.empty()) {
          xmlNodeSetName(xparam, BAD_CAST(enc->details.type_str.c_str()));
          xmlSetNs(xparam, ns);
        } else {
          xmlNodeSetName(xparam, BAD_CAST("item"));
        }
      } else {
        if (!zdata.isNull()) {
          add_xml_array_elements(xmlParam, type, enc, ns, dimension-1, dims+1,
                                 zdata, style);
        } else {
          add_xml_array_elements(xmlParam, type, enc, ns, dimension-1, dims+1,
                                 uninit_null(), style);
        }
      }
    }
  } else {
    for (int j=0; j<dims[0]; j++) {
      if (dimension == 1) {
        xmlNodePtr xparam;

        xparam = xmlNewNode(NULL, BAD_CAST("BOGUS"));
        xmlAddChild(xmlParam, xparam);
        if (type) {
          xmlNodeSetName(xparam, BAD_CAST(type->name.c_str()));
        } else if (style == SOAP_LITERAL && enc &&
                   !enc->details.type_str.empty()) {
          xmlNodeSetName(xparam, BAD_CAST(enc->details.type_str.c_str()));
          xmlSetNs(xparam, ns);
        } else {
          xmlNodeSetName(xparam, BAD_CAST("item"));
        }
      } else {
        add_xml_array_elements(xmlParam, type, enc, ns, dimension-1, dims+1,
                               uninit_null(), style);
      }
    }
  }
}

static sdlExtraAttributePtr get_extra_attributes(sdlType *sdl_type,
                                                 const char *type,
                                                 const char *wsdl_type) {
  if (sdl_type) {
    sdlAttributeMap::const_iterator iterAttributes =
      sdl_type->attributes.find(type);
    if (iterAttributes != sdl_type->attributes.end()) {
      sdlExtraAttributeMap::const_iterator iterExtraAttributes =
        iterAttributes->second->extraAttributes.find(wsdl_type);
      if (iterExtraAttributes !=
          iterAttributes->second->extraAttributes.end()) {
        return iterExtraAttributes->second;
      }
    }
  }
  return sdlExtraAttributePtr();
}

static xmlNodePtr to_xml_array(encodeTypePtr type, CVarRef data_, int style,
                               xmlNodePtr parent) {
  USE_SOAP_GLOBAL;
  sdlType *sdl_type = type->sdl_type;
  sdlTypePtr element_type;
  string array_type, array_size;
  int i;
  xmlNodePtr xmlParam;
  encodePtr enc;
  int dimension = 1;
  int* dims = NULL;
  int soap_version;
  Variant array_copy;
  Variant data = data_;

  soap_version = SOAP_GLOBAL(soap_version);

  xmlParam = xmlNewNode(NULL, BAD_CAST("BOGUS"));
  xmlAddChild(parent, xmlParam);

  if (data.isNull()) {
    if (style == SOAP_ENCODED) {
      set_xsi_nil(xmlParam);
      if (SOAP_GLOBAL(features) & SOAP_USE_XSI_ARRAY_TYPE) {
        set_ns_and_type_ex(xmlParam, (soap_version == SOAP_1_1) ?
                           SOAP_1_1_ENC_NAMESPACE : SOAP_1_2_ENC_NAMESPACE,
                           "Array");
      } else {
        set_ns_and_type(xmlParam, type);
      }
    }
    return xmlParam;
  }

  if (data.isObject() &&
      data.toObject().instanceof(SystemLib::s_IteratorClass)) {
    array_copy = Array::Create();
    for (ArrayIter iter(data.toObject().get()); iter; ++iter) {
      if (!iter.first().isNull() && iter.first().isString()) {
        array_copy.set(iter.first(), iter.second());
      } else {
        array_copy.append(iter.second());
      }
    }
    data = array_copy;
  }

  if (data.isArray()) {
    sdlExtraAttributePtr ext;
    sdlTypeMap::const_iterator iterElements;
    sdlAttributeMap::const_iterator iterAttributes;
    sdlExtraAttributeMap::const_iterator iterExtraAttributes;
    sdlTypePtr elementType;

    i = data.toArray().size();

    if ((ext = get_extra_attributes(sdl_type,
                                    SOAP_1_1_ENC_NAMESPACE":arrayType",
                                    WSDL_NAMESPACE":arrayType"))) {
      String value(ext->val);
      char *end = const_cast<char*>(strrchr(value.data(), '['));
      if (end) {
        *end = '\0';
        end++;
        dimension = calc_dimension(end);
      }
      if (!ext->ns.empty()) {
        enc = get_encoder(SOAP_GLOBAL(sdl), ext->ns.c_str(), value.data());
        get_type_str(xmlParam, ext->ns.c_str(), value.data(), array_type);
      } else {
        array_type += value.data();
      }

      dims = (int*)malloc(sizeof(int) * dimension);
      dims[0] = i;
      Array el = data.toArray();
      for (i = 1; i < dimension; i++) {
        if (!el.empty()) {
          ArrayIter iter(el);
          Variant tmp = iter.second();
          if (tmp.isArray()) {
            dims[i] = tmp.toArray().size();
          } else {
            dims[i] = 0;
          }
        }
      }

      array_size += lexical_cast<string>(dims[0]);
      for (i=1; i<dimension; i++) {
        array_size += ',';
        array_size += lexical_cast<string>(dims[i]);
      }

    } else if ((ext = get_extra_attributes
                (sdl_type,
                 SOAP_1_2_ENC_NAMESPACE":itemType",
                 WSDL_NAMESPACE":itemType"))) {
      if (!ext->ns.empty()) {
        enc = get_encoder(SOAP_GLOBAL(sdl), ext->ns.c_str(), ext->val.c_str());
        get_type_str(xmlParam, ext->ns.c_str(), ext->val.c_str(), array_type);
      } else {
        array_type += ext->val;
      }
      if ((iterAttributes =
           sdl_type->attributes.find(SOAP_1_2_ENC_NAMESPACE":arraySize")) !=
          sdl_type->attributes.end()) {
        if ((iterExtraAttributes =
             iterAttributes->second->extraAttributes.find
             (WSDL_NAMESPACE":arraySize")) !=
            iterAttributes->second->extraAttributes.end()) {
          sdlExtraAttributePtr ext = iterExtraAttributes->second;
          dimension = calc_dimension_12(ext->val.c_str());
          dims = get_position_12(dimension, ext->val.c_str());
          if (dims[0] == 0) {dims[0] = i;}

          array_size += lexical_cast<string>(dims[0]);
          for (i=1; i<dimension; i++) {
            array_size += ',';
            array_size += lexical_cast<string>(dims[i]);
          }
        }
      } else {
        dims = (int*)malloc(sizeof(int));
        *dims = 0;
        array_size += lexical_cast<string>(i);
      }
    } else if ((ext = get_extra_attributes
                (sdl_type,
                 SOAP_1_2_ENC_NAMESPACE":arraySize",
                 WSDL_NAMESPACE":arraySize"))) {
      dimension = calc_dimension_12(ext->val.c_str());
      dims = get_position_12(dimension, ext->val.c_str());
      if (dims[0] == 0) {dims[0] = i;}

      array_size += lexical_cast<string>(dims[0]);
      for (i=1; i<dimension; i++) {
        array_size += ',';
        array_size += lexical_cast<string>(dims[i]);
      }

      if (sdl_type && sdl_type->elements.size() == 1 &&
          (elementType = sdl_type->elements[0]) != NULL &&
          elementType->encode &&
          !elementType->encode->details.type_str.empty()) {
        elementType = sdl_type->elements[0];
        element_type = elementType;
        enc = elementType->encode;
        get_type_str(xmlParam, elementType->encode->details.ns.c_str(),
                     elementType->encode->details.type_str.c_str(),
                     array_type);
      } else {
        enc = get_array_type(xmlParam, data, array_type);
      }
    } else if
        (sdl_type && sdl_type->elements.size() == 1 &&
         (elementType = sdl_type->elements[0]) != NULL &&
         elementType->encode &&
         !elementType->encode->details.type_str.empty()) {
      element_type = elementType;
      enc = elementType->encode;
      get_type_str(xmlParam, elementType->encode->details.ns.c_str(),
                   elementType->encode->details.type_str.c_str(), array_type);

      array_size += lexical_cast<string>(i);
      dims = (int*)malloc(sizeof(int) * dimension);
      dims[0] = i;
    } else {

      enc = get_array_type(xmlParam, data, array_type);
      array_size += lexical_cast<string>(i);
      dims = (int*)malloc(sizeof(int) * dimension);
      dims[0] = i;
    }

    if (style == SOAP_ENCODED) {
      if (soap_version == SOAP_1_1) {
        if (array_type == "xsd:anyType") {
          array_type = "xsd:ur-type";
        }
        array_type += '[';
        array_type += array_size;
        array_type += ']';
        set_ns_prop(xmlParam, SOAP_1_1_ENC_NAMESPACE, "arrayType",
                    array_type.c_str());
      } else {
        string replaced;
        replaced.reserve(array_size.size());
        for (int i = 0; i < (int)array_size.size(); i++) {
          char ch = array_size[i];
          if (ch == ',') {
            replaced += ' ';
          } else {
            replaced += ch;
          }
        }
        set_ns_prop(xmlParam, SOAP_1_2_ENC_NAMESPACE, "itemType",
                    replaced.c_str());
        set_ns_prop(xmlParam, SOAP_1_2_ENC_NAMESPACE, "arraySize",
                    replaced.c_str());
      }
    }
    add_xml_array_elements(xmlParam, element_type, enc,
                           enc ? encode_add_ns(xmlParam,
                                               enc->details.ns.c_str()) : NULL,
                           dimension, dims, data, style);
    free(dims);
  }
  if (style == SOAP_ENCODED) {
    if (SOAP_GLOBAL(features) & SOAP_USE_XSI_ARRAY_TYPE) {
      set_ns_and_type_ex
        (xmlParam, (soap_version == SOAP_1_1) ?
         SOAP_1_1_ENC_NAMESPACE : SOAP_1_2_ENC_NAMESPACE, "Array");
    } else {
      set_ns_and_type(xmlParam, type);
    }
  }

  return xmlParam;
}

static Variant to_zval_array(encodeTypePtr type, xmlNodePtr data) {
  USE_SOAP_GLOBAL;
  Variant ret;
  xmlNodePtr trav;
  encodePtr enc;
  int dimension = 1;
  int* dims = NULL;
  int* pos = NULL;
  xmlAttrPtr attr;
  sdl *sdl;
  sdlAttributePtr arrayType;
  sdlExtraAttributePtr ext;
  sdlTypePtr elementType;

  FIND_XML_NULL(data, ret);
  sdl = SOAP_GLOBAL(sdl);

  if (data &&
      (attr = get_attribute(data->properties,"arrayType")) &&
      attr->children && attr->children->content) {
    xmlNsPtr nsptr;
    string type, ns;
    parse_namespace(attr->children->content, type, ns);
    nsptr = xmlSearchNs(attr->doc, attr->parent, NS_STRING(ns));

    String stype(type);
    char *end = const_cast<char*>(strrchr(stype.data(), '['));
    if (end) {
      *end = '\0';
      dimension = calc_dimension(end+1);
      dims = get_position(dimension, end+1);
    }
    if (nsptr != NULL) {
      enc = get_encoder(sdl, (char*)nsptr->href, stype.data());
    }

  } else if ((attr = get_attribute(data->properties,"itemType")) &&
      attr->children &&
      attr->children->content) {
    string type, ns;
    parse_namespace(attr->children->content, type, ns);
    xmlNsPtr nsptr;
    nsptr = xmlSearchNs(attr->doc, attr->parent, NS_STRING(ns));
    if (nsptr != NULL) {
      enc = get_encoder(sdl, (char*)nsptr->href, type.data());
    }

    if ((attr = get_attribute(data->properties,"arraySize")) &&
        attr->children && attr->children->content) {
      dimension = calc_dimension_12((char*)attr->children->content);
      dims = get_position_12(dimension, (char*)attr->children->content);
    } else {
      dims = (int*)malloc(sizeof(int));
      *dims = 0;
    }

  } else if ((attr = get_attribute(data->properties,"arraySize")) &&
             attr->children && attr->children->content) {

    dimension = calc_dimension_12((char*)attr->children->content);
    dims = get_position_12(dimension, (char*)attr->children->content);

  } else if ((ext = get_extra_attributes
              (type->sdl_type,
               SOAP_1_1_ENC_NAMESPACE":arrayType",
               WSDL_NAMESPACE":arrayType"))) {
    String type(ext->val);
    char *end = const_cast<char*>(strrchr(type.data(), '['));
    if (end) {
      *end = '\0';
    }
    if (!ext->ns.empty()) {
      enc = get_encoder(sdl, ext->ns.c_str(), type.data());
    }

    dims = (int*)malloc(sizeof(int));
    *dims = 0;

  } else if ((ext = get_extra_attributes
              (type->sdl_type,
               SOAP_1_2_ENC_NAMESPACE":itemType",
               WSDL_NAMESPACE":itemType"))) {
    if (!ext->ns.empty()) {
      enc = get_encoder(sdl, ext->ns.c_str(), ext->val.c_str());
    }

    if ((ext = get_extra_attributes
         (type->sdl_type,
          SOAP_1_2_ENC_NAMESPACE":arraySize",
          WSDL_NAMESPACE":arraySize"))) {
      dimension = calc_dimension_12(ext->val.c_str());
      dims = get_position_12(dimension, ext->val.c_str());
    } else {
      dims = (int*)malloc(sizeof(int));
      *dims = 0;
    }
  } else if ((ext = get_extra_attributes
              (type->sdl_type,
               SOAP_1_2_ENC_NAMESPACE":arraySize",
               WSDL_NAMESPACE":arraySize"))) {
    dimension = calc_dimension_12(ext->val.c_str());
    dims = get_position_12(dimension, ext->val.c_str());
    if (type->sdl_type && type->sdl_type->elements.size() == 1 &&
        (elementType = type->sdl_type->elements[0]) != NULL &&
        elementType->encode) {
      enc = elementType->encode;
    }
  } else if (type->sdl_type && type->sdl_type->elements.size() == 1 &&
             (elementType = type->sdl_type->elements[0]) != NULL &&
             elementType->encode) {
    enc = elementType->encode;
  }
  if (dims == NULL) {
    dimension = 1;
    dims = (int*)malloc(sizeof(int));
    *dims = 0;
  }
  pos = (int*)calloc(sizeof(int), dimension);
  if (data && (attr = get_attribute(data->properties,"offset")) &&
      attr->children && attr->children->content) {
    char* tmp = strrchr((char*)attr->children->content,'[');
    if (tmp == NULL) {
      tmp = (char*)attr->children->content;
    }
    get_position_ex(dimension, tmp, &pos);
  }

  ret = Array::Create();
  trav = data->children;
  while (trav) {
    if (trav->type == XML_ELEMENT_NODE) {
      int i;
      xmlAttrPtr position = get_attribute(trav->properties,"position");

      Variant tmpVal = master_to_zval(enc, trav);
      if (position != NULL && position->children &&
          position->children->content) {
        char* tmp = strrchr((char*)position->children->content, '[');
        if (tmp == NULL) {
          tmp = (char*)position->children->content;
        }
        get_position_ex(dimension, tmp, &pos);
      }

      /* Get/Create intermediate arrays for multidimensional arrays */
      i = 0;
      Variant *ar = &ret;
      while (i < dimension-1) {
        if (!ar->toArray().exists(pos[i])) {
          ar->set(pos[i], Array::Create());
        }
        ar = &ar->lvalAt(pos[i]);
        i++;
      }
      ar->set(pos[i], tmpVal);

      /* Increment position */
      i = dimension;
      while (i > 0) {
        i--;
        pos[i]++;
        if (pos[i] >= dims[i]) {
          if (i > 0) {
            pos[i] = 0;
          } else {
            /* TODO: Array index overflow */
          }
        } else {
          break;
        }
      }
    }
    trav = trav->next;
  }
  free(dims);
  free(pos);
  return ret;
}

/* Map encode/decode */
static xmlNodePtr to_xml_map(encodeTypePtr type, CVarRef data, int style,
                             xmlNodePtr parent) {
  xmlNodePtr xmlParam;
  int i;

  xmlParam = xmlNewNode(NULL, BAD_CAST("BOGUS"));
  xmlAddChild(parent, xmlParam);
  FIND_ZVAL_NULL(data, xmlParam, style);

  if (data.isArray()) {
    Array arr = data.toArray();
    i = arr.size();
    ArrayIter iter(arr);
    for (;i > 0;i--) {
      xmlNodePtr xparam, item;
      xmlNodePtr key;
      Variant temp_key = iter.first();
      Variant temp_data = iter.second();
      ++iter;

      item = xmlNewNode(NULL, BAD_CAST("item"));
      xmlAddChild(xmlParam, item);
      key = xmlNewNode(NULL, BAD_CAST("key"));
      xmlAddChild(item,key);
      if (temp_key.isString()) {
        if (style == SOAP_ENCODED) {
          set_xsi_type(key, "xsd:string");
        }
        xmlNodeSetContent(key, BAD_CAST(temp_key.toString().data()));
      } else {
        if (style == SOAP_ENCODED) {
          set_xsi_type(key, "xsd:int");
        }
        String skey = temp_key.toString();
        xmlNodeSetContentLen(key, BAD_CAST(skey.data()), skey.size());
      }

      xparam = master_to_xml(get_conversion(temp_data.getType()), temp_data,
                             style, item);
      xmlNodeSetName(xparam, BAD_CAST("value"));
    }
  }
  if (style == SOAP_ENCODED) {
    set_ns_and_type(xmlParam, type);
  }

  return xmlParam;
}

static Variant to_zval_map(encodeTypePtr type, xmlNodePtr data) {
  Variant ret, key, value;
  xmlNodePtr trav, item, xmlKey, xmlValue;

  FIND_XML_NULL(data, ret);

  if (data && data->children) {
    ret = Array::Create();
    trav = data->children;

    FOREACHNODE(trav, "item", item) {
      xmlKey = get_node(item->children, "key");
      if (!xmlKey) {
        throw SoapException( "Encoding: Can't decode map, missing key");
      }

      xmlValue = get_node(item->children, "value");
      if (!xmlKey) {
        throw SoapException( "Encoding: Can't decode map, missing value");
      }

      key = master_to_zval(encodePtr(), xmlKey);
      value = master_to_zval(encodePtr(), xmlValue);

      if (key.isString() || key.isInteger()) {
        ret.set(key, value);
      } else {
        throw SoapException( "Encoding: Can't decode map, only Strings or "
                       "Longs are allowd as keys");
      }
    }
    ENDFOREACH(trav);
  }
  return ret;
}

/* Unknown encode/decode */
static xmlNodePtr guess_xml_convert(encodeTypePtr type, CVarRef data,
                                    int style, xmlNodePtr parent) {
  encodePtr  enc = get_conversion(data.getType());
  xmlNodePtr ret = master_to_xml_int(enc, data, style, parent, false);
/*
  if (style == SOAP_LITERAL && SOAP_GLOBAL(sdl)) {
    set_ns_and_type(ret, &enc->details);
  }
*/
  return ret;
}

static Variant guess_zval_convert(encodeTypePtr type, xmlNodePtr data) {
  USE_SOAP_GLOBAL;
  encodePtr enc;
  xmlAttrPtr tmpattr;
  xmlChar *type_name = NULL;
  Variant ret;

  data = check_and_resolve_href(data);

  if (data == NULL) {
    enc = get_conversion(KindOfNull);
  } else if (data->properties &&
             get_attribute_ex(data->properties, "nil", XSI_NAMESPACE)) {
    enc = get_conversion(KindOfNull);
  } else {
    tmpattr = get_attribute_ex(data->properties,"type", XSI_NAMESPACE);
    if (tmpattr != NULL) {
      type_name = tmpattr->children->content;
      enc = get_encoder_from_prefix(SOAP_GLOBAL(sdl), data,
                                    tmpattr->children->content);
      if (enc && type == &enc->details) {
        enc.reset();
      }
      if (enc) {
        encodePtr tmp = enc;
        while (tmp && tmp->details.sdl_type != NULL &&
               tmp->details.sdl_type->kind != XSD_TYPEKIND_COMPLEX) {
          if (enc == tmp->details.sdl_type->encode ||
              tmp == tmp->details.sdl_type->encode) {
            enc.reset();
            break;
          }
          tmp = tmp->details.sdl_type->encode;
        }
      }
    }

    if (!enc) {
      /* Didn't have a type, totally guess here */
      /* Logic: has children = IS_OBJECT else IS_STRING */
      xmlNodePtr trav;

      if (get_attribute(data->properties, "arrayType") ||
          get_attribute(data->properties, "itemType") ||
          get_attribute(data->properties, "arraySize")) {
        enc = get_conversion(SOAP_ENC_ARRAY);
      } else {
        enc = get_conversion(XSD_STRING);
        trav = data->children;
        while (trav != NULL) {
          if (trav->type == XML_ELEMENT_NODE) {
            enc = get_conversion(SOAP_ENC_OBJECT);
            break;
          }
          trav = trav->next;
        }
      }
    }
  }
  ret = master_to_zval_int(enc, data);
  if (SOAP_GLOBAL(sdl) && type_name && enc->details.sdl_type) {
    c_SoapVar *soapvar = NEWOBJ(c_SoapVar)();
    soapvar->m_type = enc->details.type;
    soapvar->m_value = ret;

    string ns, cptype;
    parse_namespace(type_name, cptype, ns);

    xmlNsPtr nsptr = xmlSearchNs(data->doc, data, NS_STRING(ns));
    soapvar->m_stype = cptype;
    if (nsptr) {
      soapvar->m_ns = String((char*)nsptr->href, CopyString);
    }
    ret = Object(soapvar);
  }
  return ret;
}

/* Time encode/decode */
static xmlNodePtr to_xml_datetime_ex(encodeTypePtr type, CVarRef data,
                                     const char *format, int style,
                                     xmlNodePtr parent) {
  /* logic hacked from ext/standard/datetime.c */
  struct tm *ta, tmbuf;
  time_t timestamp;
  int max_reallocs = 5;
  size_t buf_len=64, real_len;
  char *buf;
  char tzbuf[8];

  xmlNodePtr xmlParam = xmlNewNode(NULL, BAD_CAST("BOGUS"));
  xmlAddChild(parent, xmlParam);
  FIND_ZVAL_NULL(data, xmlParam, style);

  if (data.isInteger()) {
    timestamp = data.toInt64();
    ta = localtime_r(&timestamp, &tmbuf);
    /*ta = php_gmtime_r(&timestamp, &tmbuf);*/

    buf = (char *)malloc(buf_len);
    while ((real_len = strftime(buf, buf_len, format, ta)) == buf_len ||
           real_len == 0) {
      buf_len *= 2;
      buf = (char *)realloc(buf, buf_len);
      if (!--max_reallocs) break;
    }

    /* Time zone support */
    snprintf(tzbuf, sizeof(tzbuf), "%c%02d:%02d",
             (ta->tm_gmtoff < 0) ? '-' : '+', (int)abs(ta->tm_gmtoff / 3600),
             (int)abs( (ta->tm_gmtoff % 3600) / 60 ));
    if (strcmp(tzbuf,"+00:00") == 0) {
      strcpy(tzbuf,"Z");
      real_len++;
    } else {
      real_len += 6;
    }
    if (real_len >= buf_len) {
      buf = (char *)realloc(buf, real_len+1);
    }
    strcat(buf, tzbuf);

    xmlNodeSetContent(xmlParam, BAD_CAST(buf));
    free(buf);
  } else if (data.isString()) {
    String sdata = data.toString();
    xmlNodeSetContentLen(xmlParam, BAD_CAST(sdata.data()), sdata.size());
  }

  if (style == SOAP_ENCODED) {
    set_ns_and_type(xmlParam, type);
  }
  return xmlParam;
}

static xmlNodePtr to_xml_duration(encodeTypePtr type, CVarRef data, int style,
                                  xmlNodePtr parent) {
  // TODO: '-'?P([0-9]+Y)?([0-9]+M)?([0-9]+D)?T([0-9]+H)?([0-9]+M)?([0-9]+S)?
  return to_xml_string(type, data, style, parent);
}

static xmlNodePtr to_xml_datetime(encodeTypePtr type, CVarRef data, int style,
                                  xmlNodePtr parent) {
  return to_xml_datetime_ex(type, data, "%Y-%m-%dT%H:%M:%S", style, parent);
}

static xmlNodePtr to_xml_time(encodeTypePtr type, CVarRef data, int style,
                              xmlNodePtr parent) {
  /* TODO: microsecconds */
  return to_xml_datetime_ex(type, data, "%H:%M:%S", style, parent);
}

static xmlNodePtr to_xml_date(encodeTypePtr type, CVarRef data, int style,
                              xmlNodePtr parent) {
  return to_xml_datetime_ex(type, data, "%Y-%m-%d", style, parent);
}

static xmlNodePtr to_xml_gyearmonth(encodeTypePtr type, CVarRef data,
                                    int style, xmlNodePtr parent) {
  return to_xml_datetime_ex(type, data, "%Y-%m", style, parent);
}

static xmlNodePtr to_xml_gyear(encodeTypePtr type, CVarRef data, int style,
                               xmlNodePtr parent) {
  return to_xml_datetime_ex(type, data, "%Y", style, parent);
}

static xmlNodePtr to_xml_gmonthday(encodeTypePtr type, CVarRef data, int style,
                                   xmlNodePtr parent) {
  return to_xml_datetime_ex(type, data, "--%m-%d", style, parent);
}

static xmlNodePtr to_xml_gday(encodeTypePtr type, CVarRef data, int style,
                              xmlNodePtr parent) {
  return to_xml_datetime_ex(type, data, "---%d", style, parent);
}

static xmlNodePtr to_xml_gmonth(encodeTypePtr type, CVarRef data, int style,
                                xmlNodePtr parent) {
  return to_xml_datetime_ex(type, data, "--%m--", style, parent);
}

static Variant to_zval_list(encodeTypePtr enc, xmlNodePtr data) {
  /*FIXME*/
  return to_zval_stringc(enc, data);
}

static xmlNodePtr to_xml_list(encodeTypePtr enc, CVarRef data, int style,
                              xmlNodePtr parent) {
  encodePtr list_enc;
  if (enc->sdl_type && enc->sdl_type->kind == XSD_TYPEKIND_LIST &&
      !enc->sdl_type->elements.empty()) {
    list_enc = enc->sdl_type->elements[0]->encode;
  }

  xmlNodePtr ret = xmlNewNode(NULL, BAD_CAST("BOGUS"));
  xmlAddChild(parent, ret);
  FIND_ZVAL_NULL(data, ret, style);
  if (data.isArray()) {
    Variant tmp;
    string list;
    Array ht = data.toArray();
    for (ArrayIter iter(ht); iter; ++iter) {
      xmlNodePtr dummy = master_to_xml(list_enc, tmp, SOAP_LITERAL, ret);
      if (dummy && dummy->children && dummy->children->content) {
        if (!list.empty()) {
          list += ' ';
        }
        list += (char*)dummy->children->content;
      } else {
        throw SoapException("Encoding: Violation of encoding rules");
      }
      xmlUnlinkNode(dummy);
      xmlFreeNode(dummy);
    }
    xmlNodeSetContentLen(ret, BAD_CAST(list.c_str()), list.size());
  } else {
    String sdata = data.toString();
    char *str = strndup(sdata.data(), sdata.size());
    whiteSpace_collapse(BAD_CAST(str));
    char *start = str;
    char *next;
    string list;
    while (start != NULL && *start != '\0') {
      xmlNodePtr dummy;
      Variant dummy_zval;

      next = strchr(start,' ');
      if (next != NULL) {
        *next = '\0';
        next++;
      }
      dummy_zval = String(start);
      dummy = master_to_xml(list_enc, dummy_zval, SOAP_LITERAL, ret);
      if (dummy && dummy->children && dummy->children->content) {
        if (!list.empty()) {
          list += ' ';
        }
        list += (char*)dummy->children->content;
      } else {
        throw SoapException("Encoding: Violation of encoding rules");
      }
      xmlUnlinkNode(dummy);
      xmlFreeNode(dummy);

      start = next;
    }
    xmlNodeSetContentLen(ret, BAD_CAST(list.c_str()), list.size());
    free(str);
  }
  return ret;
}

static xmlNodePtr to_xml_list1(encodeTypePtr enc, CVarRef data, int style,
                               xmlNodePtr parent) {
  /*FIXME: minLength=1 */
  return to_xml_list(enc,data,style, parent);
}

static Variant to_zval_union(encodeTypePtr enc, xmlNodePtr data) {
  /*FIXME*/
  return to_zval_list(enc, data);
}

static xmlNodePtr to_xml_union(encodeTypePtr enc, CVarRef data, int style,
                               xmlNodePtr parent) {
  /*FIXME*/
  return to_xml_list(enc,data,style, parent);
}

static Variant to_zval_any(encodeTypePtr type, xmlNodePtr data) {
  USE_SOAP_GLOBAL;
  xmlBufferPtr buf;
  Variant ret;

  if (SOAP_GLOBAL(sdl) && !SOAP_GLOBAL(sdl)->elements.empty() && data->name) {
    string nscat;
    if (data->ns && data->ns->href) {
      nscat += (char*)data->ns->href;
      nscat += ':';
    }
    nscat += (char*)data->name;

    sdlTypeMap::const_iterator iter = SOAP_GLOBAL(sdl)->elements.find(nscat);
    if (iter != SOAP_GLOBAL(sdl)->elements.end() && iter->second->encode) {
      return master_to_zval_int(iter->second->encode, data);
    }
  }

  buf = xmlBufferCreate();
  xmlNodeDump(buf, NULL, data, 0, 0);
  ret = String((char*)xmlBufferContent(buf), CopyString);
  xmlBufferFree(buf);
  return ret;
}

static xmlNodePtr to_xml_any(encodeTypePtr type, CVarRef data, int style,
                             xmlNodePtr parent) {
  xmlNodePtr ret = NULL;

  if (data.isArray()) {
    encodePtr enc = get_conversion(XSD_ANYXML);
    Array arr = data.toArray();
    for (ArrayIter iter(arr); iter; ++iter) {
      ret = master_to_xml(enc, iter.second(), style, parent);
      if (ret && ret->name != xmlStringTextNoenc &&
          iter.first().isString()) {
        xmlNodeSetName(ret, BAD_CAST(iter.first().toString().data()));
      }
    }
    return ret;
  }
  String sdata = data.toString();
  ret = xmlNewTextLen(BAD_CAST(sdata.data()), sdata.size());
  ret->name = xmlStringTextNoenc;
  ret->parent = parent;
  ret->doc = parent->doc;
  ret->prev = parent->last;
  ret->next = NULL;
  if (parent->last) {
    parent->last->next = ret;
  } else {
    parent->children = ret;
  }
  parent->last = ret;

  return ret;
}

Variant sdl_guess_convert_zval(encodeTypePtr enc, xmlNodePtr data) {
  sdlType *type;

  type = enc->sdl_type;
  if (type == NULL) {
    return guess_zval_convert(enc, data);
  }
/*FIXME: restriction support
  if (type && type->restrictions &&
      data &&  data->children && data->children->content) {
    if (type->restrictions->whiteSpace && type->restrictions->whiteSpace->value) {
      if (strcmp(type->restrictions->whiteSpace->value,"replace") == 0) {
        whiteSpace_replace(data->children->content);
      } else if (strcmp(type->restrictions->whiteSpace->value,"collapse") == 0) {
        whiteSpace_collapse(data->children->content);
      }
    }
    if (type->restrictions->enumeration) {
      if (!zend_hash_exists(type->restrictions->enumeration,data->children->content,strlen(data->children->content)+1)) {
        soap_error1(E_WARNING, "Encoding: Restriction: invalid enumeration value \"%s\"", data->children->content);
      }
    }
    if (type->restrictions->minLength &&
        strlen(data->children->content) < type->restrictions->minLength->value) {
      soap_error0(E_WARNING, "Encoding: Restriction: length less than 'minLength'");
    }
    if (type->restrictions->maxLength &&
        strlen(data->children->content) > type->restrictions->maxLength->value) {
      soap_error0(E_WARNING, "Encoding: Restriction: length greater than 'maxLength'");
    }
    if (type->restrictions->length &&
        strlen(data->children->content) != type->restrictions->length->value) {
      soap_error0(E_WARNING, "Encoding: Restriction: length is not equal to 'length'");
    }
  }
*/
  switch (type->kind) {
  case XSD_TYPEKIND_SIMPLE:
    if (type->encode && enc != &type->encode->details) {
      return master_to_zval_int(type->encode, data);
    } else {
      return guess_zval_convert(enc, data);
    }
    break;
  case XSD_TYPEKIND_LIST:
    return to_zval_list(enc, data);
  case XSD_TYPEKIND_UNION:
    return to_zval_union(enc, data);
  case XSD_TYPEKIND_COMPLEX:
  case XSD_TYPEKIND_RESTRICTION:
  case XSD_TYPEKIND_EXTENSION:
    if (type->encode &&
        (type->encode->details.type == KindOfArray ||
         type->encode->details.type == SOAP_ENC_ARRAY)) {
      return to_zval_array(enc, data);
    }
    return to_zval_object(enc, data);
  default:
    throw SoapException("Encoding: Internal Error");
  }
  return guess_zval_convert(enc, data);
}

xmlNodePtr sdl_guess_convert_xml(encodeTypePtr enc, CVarRef data, int style,
                                 xmlNodePtr parent) {
  sdlType *type;
  xmlNodePtr ret = NULL;

  type = enc->sdl_type;

  if (type == NULL) {
    ret = guess_xml_convert(enc, data, style, parent);
    if (style == SOAP_ENCODED) {
      set_ns_and_type(ret, enc);
    }
    return ret;
  }
/*FIXME: restriction support
  if (type) {
    if (type->restrictions && Z_TYPE_P(data) == IS_STRING) {
      if (type->restrictions->enumeration) {
        if (!zend_hash_exists(type->restrictions->enumeration,Z_STRVAL_P(data),Z_STRLEN_P(data)+1)) {
          soap_error1(E_WARNING, "Encoding: Restriction: invalid enumeration value \"%s\".", Z_STRVAL_P(data));
        }
      }
      if (type->restrictions->minLength &&
          Z_STRLEN_P(data) < type->restrictions->minLength->value) {
        soap_error0(E_WARNING, "Encoding: Restriction: length less than 'minLength'");
      }
      if (type->restrictions->maxLength &&
          Z_STRLEN_P(data) > type->restrictions->maxLength->value) {
        soap_error0(E_WARNING, "Encoding: Restriction: length greater than 'maxLength'");
      }
      if (type->restrictions->length &&
          Z_STRLEN_P(data) != type->restrictions->length->value) {
        soap_error0(E_WARNING, "Encoding: Restriction: length is not equal to 'length'");
      }
    }
  }
*/
  switch(type->kind) {
  case XSD_TYPEKIND_SIMPLE:
    if (type->encode && enc != &type->encode->details) {
      ret = master_to_xml(type->encode, data, style, parent);
    } else {
      ret = guess_xml_convert(enc, data, style, parent);
    }
    break;
  case XSD_TYPEKIND_LIST:
    ret = to_xml_list(enc, data, style, parent);
    break;
  case XSD_TYPEKIND_UNION:
    ret = to_xml_union(enc, data, style, parent);
    break;
  case XSD_TYPEKIND_COMPLEX:
  case XSD_TYPEKIND_RESTRICTION:
  case XSD_TYPEKIND_EXTENSION:
    if (type->encode &&
        (type->encode->details.type == KindOfArray ||
         type->encode->details.type == SOAP_ENC_ARRAY)) {
      return to_xml_array(enc, data, style, parent);
    } else {
      return to_xml_object(enc, data, style, parent);
    }
    break;
  default:
    throw SoapException("Encoding: Internal Error");
    break;
  }
  if (style == SOAP_ENCODED) {
    set_ns_and_type(ret, enc);
  }
  return ret;
}

static xmlNodePtr check_and_resolve_href(xmlNodePtr data) {
  if (data && data->properties) {
    xmlAttrPtr href;

    href = data->properties;
    while (1) {
      href = get_attribute(href, "href");
      if (href == NULL || href->ns == NULL) {break;}
      href = href->next;
    }
    if (href) {
      /*  Internal href try and find node */
      if (href->children->content[0] == '#') {
        xmlNodePtr ret = get_node_with_attribute_recursive
          (data->doc->children, NULL, "id",
           (char*)&href->children->content[1]);
        if (!ret) {
          throw SoapException("Encoding: Unresolved reference '%s'",
                        href->children->content);
        }
        return ret;
      } else {
        /*  TODO: External href....? */
        throw SoapException("Encoding: External reference '%s'",
                      href->children->content);
      }
    }
    /* SOAP 1.2 enc:id enc:ref */
    href = get_attribute_ex(data->properties, "ref", SOAP_1_2_ENC_NAMESPACE);
    if (href) {
      xmlChar* id;
      xmlNodePtr ret;

      if (href->children->content[0] == '#') {
        id = href->children->content+1;
      } else {
        id = href->children->content;
      }
      ret = get_node_with_attribute_recursive_ex
        (data->doc->children, NULL, NULL, "id", (char*)id,
         SOAP_1_2_ENC_NAMESPACE);
      if (!ret) {
        throw SoapException("Encoding: Unresolved reference '%s'",
                      href->children->content);
      } else if (ret == data) {
        throw SoapException("Encoding: Violation of id and ref information "
                      "items '%s'", href->children->content);
      }
      return ret;
    }
  }
  return data;
}

static void set_ns_and_type(xmlNodePtr node, encodeTypePtr type) {
  set_ns_and_type_ex(node, type->ns.c_str(), type->type_str.c_str());
}

static void set_ns_and_type_ex(xmlNodePtr node, const char *ns,
                               const char *type) {
  string nstype;
  get_type_str(node, ns, type, nstype);
  set_xsi_type(node, nstype.c_str());
}

static xmlNsPtr xmlSearchNsPrefixByHref(xmlDocPtr doc, xmlNodePtr node,
                                        const xmlChar * href) {
  xmlNsPtr cur;
  xmlNodePtr orig = node;

  while (node) {
    if (node->type == XML_ENTITY_REF_NODE ||
        node->type == XML_ENTITY_NODE ||
        node->type == XML_ENTITY_DECL) {
      return NULL;
    }
    if (node->type == XML_ELEMENT_NODE) {
      cur = node->nsDef;
      while (cur != NULL) {
        if (cur->prefix && cur->href && xmlStrEqual(cur->href, href)) {
          if (xmlSearchNs(doc, node, cur->prefix) == cur) {
            return cur;
          }
        }
        cur = cur->next;
      }
      if (orig != node) {
        cur = node->ns;
        if (cur != NULL) {
          if (cur->prefix && cur->href && xmlStrEqual(cur->href, href)) {
            if (xmlSearchNs(doc, node, cur->prefix) == cur) {
              return cur;
            }
          }
        }
      }
    }
    node = node->parent;
  }
  return NULL;
}

xmlNsPtr encode_add_ns(xmlNodePtr node, const char* ns) {
  USE_SOAP_GLOBAL;
  xmlNsPtr xmlns;

  if (ns == NULL) {
    return NULL;
  }

  xmlns = xmlSearchNsByHref(node->doc, node, BAD_CAST(ns));
  if (xmlns != NULL && xmlns->prefix == NULL) {
    xmlns = xmlSearchNsPrefixByHref(node->doc, node, BAD_CAST(ns));
  }
  if (xmlns == NULL) {
    std::map<string, string>::const_iterator iter =
      SOAP_GLOBAL(defEncNs).find(ns);
    if (iter != SOAP_GLOBAL(defEncNs).end()) {
      xmlns = xmlNewNs(node->doc->children, BAD_CAST(ns),
                       (xmlChar*)iter->second.c_str());
    } else {
      int num = ++SOAP_GLOBAL(cur_uniq_ns);

      string prefix;
      while (1) {
        prefix = "ns";
        prefix += lexical_cast<string>(num);
        if (xmlSearchNs(node->doc, node, BAD_CAST(prefix.c_str())) == NULL) {
          break;
        }
        num = ++SOAP_GLOBAL(cur_uniq_ns);
      }

      xmlns = xmlNewNs(node->doc->children, BAD_CAST(ns),
                       BAD_CAST(prefix.c_str()));
    }
  }
  return xmlns;
}

static void set_ns_prop(xmlNodePtr node, const char *ns, const char *name,
                        const char *val) {
  xmlSetNsProp(node, encode_add_ns(node, ns), BAD_CAST(name), BAD_CAST(val));
}

static void set_xsi_nil(xmlNodePtr node) {
  set_ns_prop(node, XSI_NAMESPACE, "nil", "true");
}

static void set_xsi_type(xmlNodePtr node, const char *type) {
  set_ns_prop(node, XSI_NAMESPACE, "type", type);
}

void encode_reset_ns() {
  USE_SOAP_GLOBAL;
  SOAP_GLOBAL(cur_uniq_ns) = 0;
  SOAP_GLOBAL(cur_uniq_ref) = 0;
  SOAP_GLOBAL(ref_map).clear();
}

void encode_finish() {
  USE_SOAP_GLOBAL;
  SOAP_GLOBAL(cur_uniq_ns) = 0;
  SOAP_GLOBAL(cur_uniq_ref) = 0;
  SOAP_GLOBAL(ref_map).clear();
}

encodePtr get_conversion(int encode) {
  USE_SOAP_GLOBAL;
  std::map<int, encodePtr>::const_iterator iter =
    SOAP_GLOBAL(defEncIndex).find(encode);
  if (iter != SOAP_GLOBAL(defEncIndex).end()) {
    return iter->second;
  }
  throw SoapException( "Encoding: Cannot find encoding");
}

static encodePtr get_array_type(xmlNodePtr node, CVarRef array,
                                string &type) {
  USE_SOAP_GLOBAL;
  int i, count, cur_type, prev_type;
  bool different;
  const char *prev_stype = NULL, *cur_stype = NULL, *prev_ns = NULL,
    *cur_ns = NULL;

  if (!array.isArray()) {
    type += "xsd:anyType";
    return get_conversion(XSD_ANYTYPE);
  }
  Array ht = array.toArray();

  different = false;
  cur_type = prev_type = 0;
  count = ht.size();

  ArrayIter iter(ht);
  for (i = 0;i < count;i++) {
    Variant tmp = iter.second();

    if (tmp.isObject() && tmp.toObject().instanceof(c_SoapVar::classof())) {
      c_SoapVar *var = tmp.toObject().getTyped<c_SoapVar>();
      cur_type = var->m_type;
      if (!var->m_stype.empty()) {
        cur_stype = var->m_stype.c_str();
      } else {
        cur_stype = NULL;
      }
      if (!var->m_ns.empty()) {
        cur_ns = var->m_ns.c_str();
      } else {
        cur_ns = NULL;
      }
    } else if (tmp.isArray() && !tmp.toArray()->isVectorData()) {
      cur_type = APACHE_MAP;
      cur_stype = NULL;
      cur_ns = NULL;
    } else {
      cur_type = tmp.getType();
      if (cur_type == KindOfStaticString) {
        cur_type = KindOfString;
      }
      cur_stype = NULL;
      cur_ns = NULL;
    }

    if (i > 0) {
      if ((cur_type != prev_type) ||
          (cur_stype != NULL && prev_stype != NULL &&
           strcmp(cur_stype,prev_stype) != 0) ||
          (cur_stype == NULL && cur_stype != prev_stype) ||
          (cur_ns != NULL && prev_ns != NULL && strcmp(cur_ns,prev_ns) != 0) ||
          (cur_ns == NULL && cur_ns != prev_ns)) {
        different = true;
        break;
      }
    }

    prev_type = cur_type;
    prev_stype = cur_stype;
    prev_ns = cur_ns;
    ++iter;
  }

  if (different || count == 0) {
    type += "xsd:anyType";
    return get_conversion(XSD_ANYTYPE);
  }

  encodePtr enc;
  if (cur_stype != NULL) {
     string array_type;

     if (cur_ns) {
       xmlNsPtr ns = encode_add_ns(node,cur_ns);

       type += (char*)ns->prefix;
       type += ':';
       array_type += cur_ns;
       array_type += ':';
     }
     type += cur_stype;
     array_type += cur_stype;

     enc = get_encoder_ex(SOAP_GLOBAL(sdl), array_type);
   } else {
     enc = get_conversion(cur_type);
     get_type_str(node, enc->details.ns.c_str(),
                  enc->details.type_str.c_str(), type);
  }
  return enc;
}

static void get_type_str(xmlNodePtr node, const char* ns, const char* type,
                         string &ret) {
  USE_SOAP_GLOBAL;
  if (ns) {
    xmlNsPtr xmlns;
    if (SOAP_GLOBAL(soap_version) == SOAP_1_2 &&
        strcmp(ns,SOAP_1_1_ENC_NAMESPACE) == 0) {
      ns = SOAP_1_2_ENC_NAMESPACE;
    } else if (SOAP_GLOBAL(soap_version) == SOAP_1_1 &&
               strcmp(ns,SOAP_1_2_ENC_NAMESPACE) == 0) {
      ns = SOAP_1_1_ENC_NAMESPACE;
    }
    xmlns = encode_add_ns(node,ns);
    ret += (char*)xmlns->prefix;
    ret += ':';
  }
  ret += type;
}

///////////////////////////////////////////////////////////////////////////////
}
