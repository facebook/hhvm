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

#include "hphp/runtime/ext/ext_xml.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include <expat.h>

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(xml);
///////////////////////////////////////////////////////////////////////////////

class XmlParser : public SweepableResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION(XmlParser)
  XmlParser();
  virtual ~XmlParser();
  void cleanupImpl();
  CLASSNAME_IS("xml");
  virtual const String& o_getClassNameHook() const;

  int case_folding;
  XML_Parser parser;
  XML_Char *target_encoding;

  Variant startElementHandler;
  Variant endElementHandler;
  Variant characterDataHandler;
  Variant processingInstructionHandler;
  Variant defaultHandler;
  Variant unparsedEntityDeclHandler;
  Variant notationDeclHandler;
  Variant externalEntityRefHandler;
  Variant unknownEncodingHandler;
  Variant startNamespaceDeclHandler;
  Variant endNamespaceDeclHandler;

  Variant object;

  Variant data;
  Variant info;
  int level;
  int toffset;
  int curtag;
  Variant ctag;
  char **ltags;
  int lastwasopen;
  int skipwhite;
  int isparsing;
};

XmlParser::XmlParser() : case_folding(0), parser(NULL),
    target_encoding(NULL), level(0), toffset(0), curtag(0),
    ltags(NULL), lastwasopen(0), skipwhite(0), isparsing(0) {
}

XmlParser::~XmlParser() {
  cleanupImpl();
}

void XmlParser::cleanupImpl() {
  if (parser) {
    XML_ParserFree(parser);
    parser = NULL;
  }
  if (ltags) {
    int inx;
    for (inx = 0; inx < level; inx++)
      free(ltags[inx]);
    free(ltags);
    ltags = NULL;
  }
}

void XmlParser::sweep() {
  cleanupImpl();
}

const String& XmlParser::o_getClassNameHook() const {
  return classnameof();
}

typedef struct {
  XML_Char *name;
  char (*decoding_function)(unsigned short);
  unsigned short (*encoding_function)(unsigned char);
} xml_encoding;

enum php_xml_option {
    PHP_XML_OPTION_CASE_FOLDING = 1,
    PHP_XML_OPTION_TARGET_ENCODING,
    PHP_XML_OPTION_SKIP_TAGSTART,
    PHP_XML_OPTION_SKIP_WHITE
};

static XML_Char * xml_globals_default_encoding = (XML_Char*)"UTF-8";
// for xml_parse_into_struct

#define XML_MAXLEVEL 255
// XXX this should be dynamic

#define XML(v) (xml_globals_ ## v)

inline static unsigned short xml_encode_iso_8859_1(unsigned char c) {
  return (unsigned short)c;
}

inline static char xml_decode_iso_8859_1(unsigned short c) {
  return (char)(c > 0xff ? '?' : c);
}

inline static unsigned short xml_encode_us_ascii(unsigned char c) {
  return (unsigned short)c;
}

inline static char xml_decode_us_ascii(unsigned short c) {
  return (char)(c > 0x7f ? '?' : c);
}

xml_encoding xml_encodings[] = {
  { (XML_Char*)"ISO-8859-1", xml_decode_iso_8859_1, xml_encode_iso_8859_1 },
  { (XML_Char*)"US-ASCII",   xml_decode_us_ascii,   xml_encode_us_ascii   },
  { (XML_Char*)"UTF-8",      NULL,                  NULL                  },
  { (XML_Char*)NULL,         NULL,                  NULL                  }
};

static void *php_xml_malloc_wrapper(size_t sz) {
  return malloc(sz);
}

static void *php_xml_realloc_wrapper(void *ptr, size_t sz) {
  return realloc(ptr, sz);
}

static void php_xml_free_wrapper(void *ptr) {
  if (ptr != NULL) {
    free(ptr);
  }
}

static XML_Memory_Handling_Suite php_xml_mem_hdlrs = {
  php_xml_malloc_wrapper,
  php_xml_realloc_wrapper,
  php_xml_free_wrapper
};

static xml_encoding *xml_get_encoding(const XML_Char *name) {
  xml_encoding *enc = &xml_encodings[0];

  while (enc && enc->name) {
    if (strcasecmp((const char*)name, (const char*)enc->name) == 0) {
      return enc;
    }
    enc++;
  }
  return NULL;
}

static int _xml_xmlcharlen(const XML_Char *s) {
  int len = 0;

  while (*s) {
    len++;
    s++;
  }
  return len;
}

char *xml_utf8_decode(const XML_Char *s, int len, int *newlen,
                      const XML_Char *encoding) {
  int pos = len;
  char *newbuf = (char*)malloc(len+1);
  unsigned short c;
  char (*decoder)(unsigned short) = NULL;
  xml_encoding *enc = xml_get_encoding(encoding);

  *newlen = 0;
  if (enc) {
    decoder = enc->decoding_function;
  }
  if (decoder == NULL) {
    /* If the target encoding was unknown, or no decoder function
     * was specified, return the UTF-8-encoded data as-is.
     */
    memcpy(newbuf, s, len);
    *newlen = len;
    newbuf[*newlen] = '\0';
    return newbuf;
  }
  while (pos > 0) {
    c = (unsigned char)(*s);
    if (c >= 0xf0) { /* four bytes encoded, 21 bits */
      if(pos-4 >= 0) {
        c = ((s[0]&7)<<18) | ((s[1]&63)<<12) | ((s[2]&63)<<6) | (s[3]&63);
      } else {
        c = '?';
      }
      s += 4;
      pos -= 4;
    } else if (c >= 0xe0) { /* three bytes encoded, 16 bits */
      if(pos-3 >= 0) {
        c = ((s[0]&63)<<12) | ((s[1]&63)<<6) | (s[2]&63);
      } else {
        c = '?';
      }
      s += 3;
      pos -= 3;
    } else if (c >= 0xc0) { /* two bytes encoded, 11 bits */
      if(pos-2 >= 0) {
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
    newbuf[*newlen] = decoder ? decoder(c) : c;
    ++*newlen;
  }
  if (*newlen < len) {
    newbuf = (char*)realloc(newbuf, *newlen + 1);
  }
  newbuf[*newlen] = '\0';
  return newbuf;
}

static Variant _xml_xmlchar_zval(const XML_Char *s, int len,
                                 const XML_Char *encoding) {
  if (s == NULL) {
    return false;
  }
  if (len == 0) {
    len = _xml_xmlcharlen(s);
  }
  int ret_len;
  char * ret = xml_utf8_decode(s, len, &ret_len, encoding);
  return String(ret, ret_len, AttachString);
}

static char *_xml_decode_tag(XmlParser *parser, const char *tag) {
  char *newstr;
  int out_len;
  newstr = xml_utf8_decode((const XML_Char*)tag, strlen(tag), &out_len,
                           parser->target_encoding);
  if (parser->case_folding) {
    char* oldstr = newstr;
    newstr = string_to_upper(oldstr, out_len);
    free(oldstr);
  }
  return newstr;
}

static Variant php_xml_parser_create_impl(const String& encoding_param,
                                          const String& ns_param, int ns_support) {
  XmlParser *parser;
  int auto_detect = 0;
  XML_Char *encoding;

  if (!encoding_param.isNull()) {
    /* The supported encoding types are hardcoded here because
     * we are limited to the encodings supported by expat/xmltok.
     */
    if (encoding_param.size() == 0) {
      encoding = XML(default_encoding);
      auto_detect = 1;
    } else if (strcasecmp(encoding_param.data(), "ISO-8859-1") == 0) {
      encoding = (XML_Char*)"ISO-8859-1";
    } else if (strcasecmp(encoding_param.data(), "UTF-8") == 0) {
      encoding = (XML_Char*)"UTF-8";
    } else if (strcasecmp(encoding_param.data(), "US-ASCII") == 0) {
      encoding = (XML_Char*)"US-ASCII";
    } else {
      raise_warning("unsupported source encoding \"%s\"",
                    encoding_param.c_str());
      return false;
    }
  } else {
    encoding = XML(default_encoding);
  }

  String separator;
  if (ns_support && ns_param.empty()){
    separator = ":";
  } else {
    separator = ns_param;
  }

  parser = NEWOBJ(XmlParser)();
  parser->parser = XML_ParserCreate_MM
    ((auto_detect ? NULL : encoding), &php_xml_mem_hdlrs,
     !separator.empty() ? (const XML_Char*)separator.data() : NULL);

  parser->target_encoding = encoding;
  parser->case_folding = 1;
  parser->object.reset();
  parser->isparsing = 0;

  XML_SetUserData(parser->parser, parser);

  return Resource(parser);
}

static String _xml_string_zval(const char *str) {
  return String(str, CopyString);
}

static Variant xml_call_handler(XmlParser *parser, CVarRef handler,
                                CArrRef args) {
  if (parser && handler.toBoolean()) {
    Variant retval;
    if (handler.isString()) {
      if (!parser->object.isObject()) {
        retval = invoke(handler.toString().c_str(), args, -1);
      } else {
        retval = parser->object.toObject()->
          o_invoke(handler.toString(), args);
      }
    } else if (handler.isArray() && handler.getArrayData()->size() == 2 &&
               (handler[0].isString() || handler[0].isObject()) &&
               handler[1].isString()) {
      vm_call_user_func(handler, args);
    } else {
      raise_warning("Handler is invalid");
    }
    return retval;
  }
  return uninit_null();
}

static void _xml_add_to_info(XmlParser *parser, char *name) {
  if (parser->info.isNull()) {
    return;
  }
  String nameStr(name, CopyString);
  if (!parser->info.toArray().exists(nameStr)) {
    parser->info.set(nameStr, Array::Create());
  }
  parser->info.lvalAt(nameStr).append(parser->curtag);
  parser->curtag++;
}

const StaticString
  s_type("type"),
  s_complete("complete"),
  s_tag("tag"),
  s_close("close"),
  s_level("level"),
  s_value("value"),
  s_cdata("cdata"),
  s_open("open"),
  s_attributes("attributes");

void _xml_endElementHandler(void *userData, const XML_Char *name) {
  XmlParser *parser = (XmlParser *)userData;
  char *tag_name;

  if (parser) {
    Variant retval;
    Array args = Array::Create();

    tag_name = _xml_decode_tag(parser, (const char*)name);

    if (parser->endElementHandler.toBoolean()) {
      args.append(parser);
      args.append(_xml_string_zval(tag_name));
      xml_call_handler(parser, parser->endElementHandler, args);
    }

    if (!parser->data.isNull()) {
      if (parser->lastwasopen) {
        parser->ctag.set(s_type, s_complete);
      } else {
        ArrayInit tag(3);
        _xml_add_to_info(parser,((char*)tag_name) + parser->toffset);
        tag.set(s_tag, String(((char*)tag_name) + parser->toffset, CopyString));
        tag.set(s_type, s_close);
        tag.set(s_level, parser->level);
        parser->data.append(tag.create());
      }
      parser->lastwasopen = 0;
    }

    free(tag_name);

    if (parser->ltags) {
      free(parser->ltags[parser->level-1]);
    }

    parser->level--;
  }
}

void _xml_characterDataHandler(void *userData, const XML_Char *s, int len) {
  XmlParser *parser = (XmlParser *)userData;

  if (parser) {
    Variant retval;
    Array args = Array::Create();

    if (parser->characterDataHandler.toBoolean()) {
      args.append(parser);
      args.append(_xml_xmlchar_zval(s, len, parser->target_encoding));
      xml_call_handler(parser, parser->characterDataHandler, args);
    }

    if (!parser->data.isNull()) {
      int i;
      int doprint = 0;

      char *decoded_value;
      int decoded_len;
      decoded_value = xml_utf8_decode(s,len,&decoded_len,
                                      parser->target_encoding);
      for (i = 0; i < decoded_len; i++) {
        switch (decoded_value[i]) {
        case ' ':
        case '\t':
        case '\n':
          continue;
        default:
          doprint = 1;
          break;
        }
        if (doprint) {
          break;
        }
      }
      if (doprint || (! parser->skipwhite)) {
        if (parser->lastwasopen) {
          String myval;
          // check if value exists, if yes append to that
          if (parser->ctag.toArray().exists(s_value))
          {
            myval = parser->ctag.rvalAt(s_value).toString();
            myval += String(decoded_value, decoded_len, AttachString);
            parser->ctag.set(s_value, myval);
          } else {
            parser->ctag.set(s_value,
                             String(decoded_value,decoded_len,AttachString));
          }
        } else {
          Array tag;
          Variant curtag;
          String myval;
          String mytype;
          curtag.assignRef(parser->data.getArrayData()->endRef());
          if (curtag.toArray().exists(s_type)) {
            mytype = curtag.rvalAt(s_type).toString();
            if (!strcmp(mytype.data(), "cdata") &&
                curtag.toArray().exists(s_value)) {
              myval = curtag.rvalAt(s_value).toString();
              myval += String(decoded_value, decoded_len, AttachString);
              curtag.set(s_value, myval);
              return;
            }
          }
          tag = Array::Create();
          _xml_add_to_info(parser, parser->ltags[parser->level-1] +
                           parser->toffset);
          tag.set(s_tag, String(parser->ltags[parser->level-1] +
                                parser->toffset, CopyString));
          tag.set(s_value, String(decoded_value, AttachString));
          tag.set(s_type, s_cdata);
          tag.set(s_level, parser->level);
          parser->data.append(tag);
        }
      } else {
        free(decoded_value);
      }
    }
  }
}

void _xml_defaultHandler(void *userData, const XML_Char *s, int len) {
  XmlParser *parser = (XmlParser *)userData;

  if (parser && parser->defaultHandler.toBoolean()) {
    xml_call_handler(parser, parser->defaultHandler, make_packed_array(
        parser, _xml_xmlchar_zval(s, len, parser->target_encoding)));
  }
}

void _xml_startElementHandler(void *userData, const XML_Char *name, const XML_Char **attributes) {
  XmlParser *parser = (XmlParser *)userData;
  const char **attrs = (const char **) attributes;
  Variant retval;
  Array args = Array::Create();

  if (parser) {
    parser->level++;

    char* tag_name = _xml_decode_tag(parser, (const char*)name);

    if (parser->startElementHandler.toBoolean()) {
      args.append(parser);
      args.append(_xml_string_zval(tag_name));
      args.append(Array::Create());

      while (attributes && *attributes) {
        char* att = _xml_decode_tag(parser, (const char*)attributes[0]);
        int val_len;
        char* val = xml_utf8_decode(attributes[1],
                                    strlen((const char*)attributes[1]),
                                    &val_len, parser->target_encoding);
        args.lvalAt(2).set(String(att, AttachString),
                           String(val, val_len, AttachString));
        attributes += 2;
      }

      xml_call_handler(parser, parser->startElementHandler, args);
    }

    if (!parser->data.isNull()) {
      Array tag, atr;
      int atcnt = 0;
      tag = Array::Create();
      atr = Array::Create();

      _xml_add_to_info(parser,((char *) tag_name) + parser->toffset);

      tag.set(s_tag,String(((char *)tag_name)+parser->toffset,CopyString));
      tag.set(s_type, s_open);
      tag.set(s_level, parser->level);

      parser->ltags[parser->level-1] = strdup(tag_name);
      parser->lastwasopen = 1;

      attributes = (const XML_Char **) attrs;

      while (attributes && *attributes) {
        char* att = _xml_decode_tag(parser, (const char*)attributes[0]);
        int val_len;
        char* val = xml_utf8_decode(attributes[1],
                                    strlen((const char*)attributes[1]),
                                    &val_len, parser->target_encoding);
        atr.set(String(att, AttachString), String(val, val_len, AttachString));
        atcnt++;
        attributes += 2;
      }

      if (atcnt) {
        tag.set(s_attributes,atr);
      }
      parser->data.append(tag);
      parser->ctag.assignRef(parser->data.getArrayData()->endRef());
    }

    free(tag_name);
  }
}

void _xml_processingInstructionHandler(void *userData, const XML_Char *target,
                                       const XML_Char *data) {
  XmlParser *parser = (XmlParser *)userData;
  if (parser && parser->processingInstructionHandler.toBoolean()) {
    Array args = Array::Create();
    args.append(parser);
    args.append(_xml_xmlchar_zval(target, 0, parser->target_encoding));
    args.append(_xml_xmlchar_zval(data, 0, parser->target_encoding));
    xml_call_handler(parser, parser->processingInstructionHandler, args);
  }
}

int _xml_externalEntityRefHandler(XML_ParserStruct* /* void* */ parserPtr,
                                  const XML_Char *openEntityNames,
                                  const XML_Char *base,
                                  const XML_Char *systemId,
                                  const XML_Char *publicId) {
  XmlParser *parser = (XmlParser*)XML_GetUserData((XML_Parser)parserPtr);
  int ret = 0; /* abort if no handler is set (should be configurable?) */
  if (parser && parser->externalEntityRefHandler.toBoolean()) {
    Array args = Array::Create();
    args.append(parser);
    args.append(_xml_xmlchar_zval(openEntityNames, 0,
                                  parser->target_encoding));
    args.append(_xml_xmlchar_zval(base, 0, parser->target_encoding));
    args.append(_xml_xmlchar_zval(systemId, 0, parser->target_encoding));
    args.append(_xml_xmlchar_zval(publicId, 0, parser->target_encoding));
    ret = xml_call_handler(parser,
      parser->externalEntityRefHandler, args).toInt64();
  }
  return ret;
}

void _xml_notationDeclHandler(void *userData,
                              const XML_Char *notationName,
                              const XML_Char *base,
                              const XML_Char *systemId,
                              const XML_Char *publicId) {
  XmlParser *parser = (XmlParser *)userData;

  if (parser && parser->notationDeclHandler.toBoolean()) {
    Array args = Array::Create();
    args.append(parser);
    args.append(_xml_xmlchar_zval(notationName, 0, parser->target_encoding));
    args.append(_xml_xmlchar_zval(base, 0, parser->target_encoding));
    args.append(_xml_xmlchar_zval(systemId, 0, parser->target_encoding));
    args.append(_xml_xmlchar_zval(publicId, 0, parser->target_encoding));
    xml_call_handler(parser, parser->notationDeclHandler, args);
  }
}

void _xml_startNamespaceDeclHandler(void *userData,const XML_Char *prefix,
                                    const XML_Char *uri) {
  XmlParser *parser = (XmlParser *)userData;

  if (parser && parser->startNamespaceDeclHandler.toBoolean()) {
    Array args = Array::Create();

    args.append(parser);
    args.append(_xml_xmlchar_zval(prefix, 0, parser->target_encoding));
    args.append(_xml_xmlchar_zval(uri, 0, parser->target_encoding));
    xml_call_handler(parser, parser->startNamespaceDeclHandler, args);
  }
}

void _xml_endNamespaceDeclHandler(void *userData, const XML_Char *prefix) {
  XmlParser *parser = (XmlParser *)userData;

  if (parser && parser->endNamespaceDeclHandler.toBoolean()) {
    Array args = Array::Create();
    args.append(parser);
    args.append(_xml_xmlchar_zval(prefix, 0, parser->target_encoding));
    xml_call_handler(parser, parser->endNamespaceDeclHandler, args);
  }
}

void _xml_unparsedEntityDeclHandler(void *userData,
                                    const XML_Char *entityName,
                                    const XML_Char *base,
                                    const XML_Char *systemId,
                                    const XML_Char *publicId,
                                    const XML_Char *notationName) {
  XmlParser *parser = (XmlParser *)userData;

  if (parser && parser->unparsedEntityDeclHandler.toBoolean()) {
    Array args = Array::Create();
    args.append(parser);
    args.append(_xml_xmlchar_zval(entityName, 0, parser->target_encoding));
    args.append(_xml_xmlchar_zval(base, 0, parser->target_encoding));
    args.append(_xml_xmlchar_zval(systemId, 0, parser->target_encoding));
    args.append(_xml_xmlchar_zval(publicId, 0, parser->target_encoding));
    args.append(_xml_xmlchar_zval(notationName, 0, parser->target_encoding));
    xml_call_handler(parser, parser->unparsedEntityDeclHandler, args);
  }
}

static void xml_set_handler(Variant * handler, CVarRef data) {
  if (same(data, false) || data.isString() ||
      (data.isArray() && data.getArrayData()->size() == 2 &&
       (data[0].isString() || data[0].isObject()) &&
       data[1].isString())) {
    *handler = data;
  } else {
    raise_warning("Handler is invalid");
  }
}

///////////////////////////////////////////////////////////////////////////////

Resource f_xml_parser_create(const String& encoding /* = null_string */) {
  return php_xml_parser_create_impl(encoding, null_string, 0).toResource();
}

Resource f_xml_parser_create_ns(const String& encoding /* = null_string */,
                              const String& separator /* = null_string */) {
  return php_xml_parser_create_impl(encoding, separator, 1).toResource();
}

bool f_xml_parser_free(CResRef parser) {
  XmlParser * p = parser.getTyped<XmlParser>();
  if (p->isparsing == 1) {
    raise_warning("Parser cannot be freed while it is parsing.");
    return false;
  }
  return true;
}

int64_t f_xml_parse(CResRef parser, const String& data, bool is_final /* = true */) {
  // XML_Parse can reenter the VM, and it will do so after we've lost
  // the frame pointer by calling through the system's copy of XML_Parse
  // in libexpat.so.
  SYNC_VM_REGS_SCOPED();
  XmlParser * p = parser.getTyped<XmlParser>();
  int ret;
  long isFinal = is_final ? 1 : 0;
  p->isparsing = 1;
  ret = XML_Parse(p->parser, (const XML_Char*)data.data(), data.size(),
                  isFinal);
  p->isparsing = 0;
  return ret;
}

int64_t f_xml_parse_into_struct(CResRef parser, const String& data, VRefParam values,
                            VRefParam index /* = null */) {
  int ret;
  XmlParser * p = parser.getTyped<XmlParser>();
  values = Array::Create();
  p->data.assignRef(values);
  index = Array::Create();
  p->info.assignRef(index);
  p->level = 0;
  p->ltags = (char**)malloc(XML_MAXLEVEL * sizeof(char*));

  XML_SetDefaultHandler(p->parser, _xml_defaultHandler);
  XML_SetElementHandler(p->parser, _xml_startElementHandler,
                        _xml_endElementHandler);
  XML_SetCharacterDataHandler(p->parser, _xml_characterDataHandler);

  p->isparsing = 1;
  ret = XML_Parse(p->parser, (const XML_Char*)data.data(), data.size(), 1);
  p->isparsing = 0;

  return ret;
}

Variant f_xml_parser_get_option(CResRef parser, int option) {
  XmlParser * p = parser.getTyped<XmlParser>();
  switch (option) {
  case PHP_XML_OPTION_CASE_FOLDING:
    return p->case_folding;
  case PHP_XML_OPTION_TARGET_ENCODING:
    return String((const char*)p->target_encoding, CopyString);
  default:
    raise_warning("Unknown option");
    return false;
  }
  return false;
}

bool f_xml_parser_set_option(CResRef parser, int option, CVarRef value) {
  XmlParser * p = parser.getTyped<XmlParser>();
  switch (option) {
  case PHP_XML_OPTION_CASE_FOLDING:
    p->case_folding = value.toInt64();
    break;
  case PHP_XML_OPTION_SKIP_TAGSTART:
    p->toffset = value.toInt64();
    break;
  case PHP_XML_OPTION_SKIP_WHITE:
    p->skipwhite = value.toInt64();
    break;
  case PHP_XML_OPTION_TARGET_ENCODING: {
    xml_encoding *enc;
    enc = xml_get_encoding((const XML_Char*)value.toString().data());
    if (enc == NULL) {
      raise_warning("Unsupported target encoding \"%s\"",
                    value.toString().data());
      return false;
    }
    p->target_encoding = enc->name;
    break;
  }
  default:
    raise_warning("Unknown option");
    return false;
  }
  return true;
}

bool f_xml_set_character_data_handler(CResRef parser, CVarRef handler) {
  XmlParser * p = parser.getTyped<XmlParser>();
  xml_set_handler(&p->characterDataHandler, handler);
  XML_SetCharacterDataHandler(p->parser, _xml_characterDataHandler);
  return true;
}

bool f_xml_set_default_handler(CResRef parser, CVarRef handler) {
  XmlParser * p = parser.getTyped<XmlParser>();
  xml_set_handler(&p->defaultHandler, handler);
  XML_SetDefaultHandler(p->parser, _xml_defaultHandler);
  return true;
}

bool f_xml_set_element_handler(CResRef parser, CVarRef start_element_handler,
                               CVarRef end_element_handler) {
  XmlParser * p = parser.getTyped<XmlParser>();
  xml_set_handler(&p->startElementHandler, start_element_handler);
  xml_set_handler(&p->endElementHandler, end_element_handler);
  XML_SetElementHandler(p->parser, _xml_startElementHandler,
                        _xml_endElementHandler);
  return true;
}

bool f_xml_set_processing_instruction_handler(CResRef parser, CVarRef handler){
  XmlParser * p = parser.getTyped<XmlParser>();
  xml_set_handler(&p->processingInstructionHandler, handler);
  XML_SetProcessingInstructionHandler(p->parser,
                                      _xml_processingInstructionHandler);
  return true;
}

bool f_xml_set_start_namespace_decl_handler(CResRef parser, CVarRef handler) {
  XmlParser * p = parser.getTyped<XmlParser>();
  xml_set_handler(&p->startNamespaceDeclHandler, handler);
  XML_SetStartNamespaceDeclHandler(p->parser, _xml_startNamespaceDeclHandler);
  return true;
}

bool f_xml_set_end_namespace_decl_handler(CResRef parser, CVarRef handler) {
  XmlParser * p = parser.getTyped<XmlParser>();
  xml_set_handler(&p->endNamespaceDeclHandler, handler);
  XML_SetEndNamespaceDeclHandler(p->parser, _xml_endNamespaceDeclHandler);
  return true;
}

bool f_xml_set_unparsed_entity_decl_handler(CResRef parser, CVarRef handler) {
  XmlParser * p = parser.getTyped<XmlParser>();
  xml_set_handler(&p->unparsedEntityDeclHandler, handler);
  XML_SetUnparsedEntityDeclHandler(p->parser, _xml_unparsedEntityDeclHandler);
  return true;
}

bool f_xml_set_external_entity_ref_handler(CResRef parser, CVarRef handler) {
  XmlParser * p = parser.getTyped<XmlParser>();
  xml_set_handler(&p->externalEntityRefHandler, handler);
  XML_SetExternalEntityRefHandler(p->parser, _xml_externalEntityRefHandler);
  return true;
}

bool f_xml_set_notation_decl_handler(CResRef parser, CVarRef handler) {
  XmlParser * p = parser.getTyped<XmlParser>();
  xml_set_handler(&p->notationDeclHandler, handler);
  XML_SetNotationDeclHandler(p->parser, _xml_notationDeclHandler);
  return true;
}

bool f_xml_set_object(CResRef parser, VRefParam object) {
  XmlParser * p = parser.getTyped<XmlParser>();
  p->object.assignRef(object);
  return true;
}

int64_t f_xml_get_current_byte_index(CResRef parser) {
  XmlParser * p = parser.getTyped<XmlParser>();
  return XML_GetCurrentByteIndex(p->parser);
}

int64_t f_xml_get_current_column_number(CResRef parser) {
  XmlParser * p = parser.getTyped<XmlParser>();
  return XML_GetCurrentColumnNumber(p->parser);
}

int64_t f_xml_get_current_line_number(CResRef parser) {
  XmlParser * p = parser.getTyped<XmlParser>();
  return XML_GetCurrentLineNumber(p->parser);
}

int64_t f_xml_get_error_code(CResRef parser) {
  XmlParser * p = parser.getTyped<XmlParser>();
  return XML_GetErrorCode(p->parser);
}

String f_xml_error_string(int code) {
  char * str = (char *)XML_ErrorString((XML_Error)/*(int)*/code);
  return String(str, CopyString);
}

///////////////////////////////////////////////////////////////////////////////

String f_utf8_decode(const String& data) {
  String str = String(data.size(), ReserveString);
  char *newbuf = str.bufferSlice().ptr;
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
  return str.setSize(newlen);
}

String f_utf8_encode(const String& data) {
  String str = String(data.size() * 4, ReserveString);
  char *newbuf = str.bufferSlice().ptr;
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
  return str.setSize(newlen);
}

///////////////////////////////////////////////////////////////////////////////
}
