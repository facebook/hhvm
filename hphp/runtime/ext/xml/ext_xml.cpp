/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/ext/xml/ext_xml.h"

#include <folly/ScopeGuard.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/base/utf8-decode.h"
#include <expat.h>

#define XML_MAXLEVEL 255
// XXX this should be dynamic

namespace HPHP {

static class XMLExtension final : public Extension {
public:
  XMLExtension() : Extension("xml", NO_EXTENSION_VERSION_YET) {}
  void moduleInit() override {
    HHVM_FE(xml_parser_create);
    HHVM_FE(xml_parser_free);
    HHVM_FE(xml_parse);
    HHVM_FE(xml_parse_into_struct);
    HHVM_FE(xml_parser_create_ns);
    HHVM_FE(xml_parser_get_option);
    HHVM_FE(xml_parser_set_option);
    HHVM_FE(xml_set_character_data_handler);
    HHVM_FE(xml_set_default_handler);
    HHVM_FE(xml_set_element_handler);
    HHVM_FE(xml_set_processing_instruction_handler);
    HHVM_FE(xml_set_start_namespace_decl_handler);
    HHVM_FE(xml_set_end_namespace_decl_handler);
    HHVM_FE(xml_set_unparsed_entity_decl_handler);
    HHVM_FE(xml_set_external_entity_ref_handler);
    HHVM_FE(xml_set_notation_decl_handler);
    HHVM_FE(xml_set_object);
    HHVM_FE(xml_get_current_byte_index);
    HHVM_FE(xml_get_current_column_number);
    HHVM_FE(xml_get_current_line_number);
    HHVM_FE(xml_get_error_code);
    HHVM_FE(xml_error_string);
    HHVM_FE(utf8_decode);
    HHVM_FE(utf8_encode);

    loadSystemlib();
  }
} s_xml_extension;

///////////////////////////////////////////////////////////////////////////////

class XmlParser : public SweepableResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION(XmlParser)
  XmlParser() {}
  virtual ~XmlParser();
  void cleanupImpl();
  CLASSNAME_IS("xml");
  const String& o_getClassNameHook() const override;

  int case_folding{0};
  XML_Parser parser{nullptr};
  XML_Char *target_encoding{nullptr};

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
  int level{0};
  int toffset{0};
  int curtag{0};
  Variant ctag;
  char **ltags{nullptr};
  int lastwasopen{0};
  int skipwhite{0};
  int isparsing{0};
};

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
    for (inx = 0; (inx < level) && (inx < XML_MAXLEVEL); inx++)
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

namespace {

inline req::ptr<XmlParser> getParserFromToken(void* userData) {
  auto token = reinterpret_cast<MemoryManager::RootId>(userData);
  return MM().lookupRoot<XmlParser>(token);
}

inline void* getParserToken(const req::ptr<XmlParser>& parser) {
  return reinterpret_cast<void*>(MM().addRoot(parser));
}

inline void clearParser(const req::ptr<XmlParser>& p) {
  MM().removeRoot(p);
}

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
  return req::malloc(sz);
}

static void *php_xml_realloc_wrapper(void *ptr, size_t sz) {
  return req::realloc(ptr, sz);
}

static void php_xml_free_wrapper(void *ptr) {
  if (ptr) {
    req::free(ptr);
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

String xml_utf8_decode(const XML_Char *s, int len,
                       const XML_Char *encoding) {
  String str = String(len, ReserveString);
  char *newbuf = str.mutableData();
  char (*decoder)(unsigned short) = nullptr;
  xml_encoding *enc = xml_get_encoding(encoding);

  int newlen = 0;
  if (enc) {
    decoder = enc->decoding_function;
  }
  if (decoder == nullptr) {
    /* If the target encoding was unknown, or no decoder function
     * was specified, return the UTF-8-encoded data as-is.
     */
    memcpy(newbuf, s, len);
    str.setSize(len);
    return str;
  }

  UTF8To16Decoder dec(s, len, true);
  for (int b = dec.decode(); b != UTF8_END; b = dec.decode()) {
    newbuf[newlen] = decoder(b);
    ++newlen;
  }

  assert(newlen <= len);
  str.shrink(newlen);
  return str;
}

static Variant _xml_xmlchar_zval(const XML_Char *s, int len,
                                 const XML_Char *encoding) {
  if (s == NULL) {
    return false;
  }
  if (len == 0) {
    len = _xml_xmlcharlen(s);
  }
  String ret = xml_utf8_decode(s, len, encoding);
  return ret;
}

static
String _xml_decode_tag(const req::ptr<XmlParser>& parser, const char *tag) {
  auto newstr = xml_utf8_decode((const XML_Char*)tag, strlen(tag),
                                parser->target_encoding);
  if (parser->case_folding) {
    string_to_upper(newstr);
  }
  return newstr;
}

static Variant php_xml_parser_create_impl(const String& encoding_param,
                                          const String& ns_param,
                                          int ns_support) {
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
  if (ns_support && ns_param.empty()) {
    separator = ":";
  } else {
    separator = ns_param;
  }

  auto parser = req::make<XmlParser>();
  parser->parser = XML_ParserCreate_MM
    ((auto_detect ? NULL : encoding), &php_xml_mem_hdlrs,
     !separator.empty() ? (const XML_Char*)separator.data() : NULL);

  parser->target_encoding = encoding;
  parser->case_folding = 1;
  parser->object.asTypedValue()->m_type = KindOfNull;
  parser->isparsing = 0;

  XML_SetUserData(parser->parser, getParserToken(parser));

  return Variant(std::move(parser));
}

static Variant xml_call_handler(const req::ptr<XmlParser>& parser,
                                const Variant& handler,
                                const Array& args) {
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
               (handler.toCArrRef()[0].isString() ||
                handler.toCArrRef()[0].isObject()) &&
               handler.toCArrRef()[1].isString()) {
      vm_call_user_func(handler, args);
    } else {
      raise_warning("Handler is invalid");
    }
    return retval;
  }
  return init_null();
}

static void _xml_add_to_info(const req::ptr<XmlParser>& parser,
                             const String& nameStr) {
  if (parser->info.isNull()) {
    return;
  }
  forceToArray(parser->info);
  if (!parser->info.toCArrRef().exists(nameStr)) {
    parser->info.toArrRef().set(nameStr, Array::Create());
  }
  auto& inner = parser->info.toArrRef().lvalAt(nameStr);
  forceToArray(inner).append(parser->curtag);
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
  auto parser = getParserFromToken(userData);

  if (parser) {
    Variant retval;
    Array args = Array::Create();

    auto tag_name = _xml_decode_tag(parser, (const char*)name);

    if (parser->endElementHandler.toBoolean()) {
      args.append(Variant(parser));
      args.append(tag_name);
      xml_call_handler(parser, parser->endElementHandler, args);
    }

    if (!parser->data.isNull()) {
      if (parser->lastwasopen) {
        parser->ctag.toArrRef().set(s_type, s_complete);
      } else {
        ArrayInit tag(3, ArrayInit::Map{});
        _xml_add_to_info(parser, tag_name.substr(parser->toffset));
        tag.set(s_tag, tag_name.substr(parser->toffset));
        tag.set(s_type, s_close);
        tag.set(s_level, parser->level);
        parser->data.toArrRef().append(tag.toArray());
      }
      parser->lastwasopen = 0;
    }


    if ((parser->ltags) && (parser->level <= XML_MAXLEVEL)) {
      free(parser->ltags[parser->level-1]);
    }

    parser->level--;
  }
}

void _xml_characterDataHandler(void *userData, const XML_Char *s, int len) {
  auto parser = getParserFromToken(userData);

  if (parser) {
    Variant retval;
    Array args = Array::Create();

    if (parser->characterDataHandler.toBoolean()) {
      args.append(Variant(parser));
      args.append(_xml_xmlchar_zval(s, len, parser->target_encoding));
      xml_call_handler(parser, parser->characterDataHandler, args);
    }

    if (!parser->data.isNull()) {
      int i;
      int doprint = 0;

      String decoded_value;
      int decoded_len;
      decoded_value = xml_utf8_decode(s,len,
                                          parser->target_encoding);
      decoded_len = decoded_value.size();
      for (i = 0; i < decoded_len; i++) {
        switch (decoded_value[i]) {
        case ' ':
        case '\t':
        case '\n':
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
          if (parser->ctag.toArrRef().exists(s_value)) {
            myval = parser->ctag.toArray().rvalAt(s_value).toString();
            myval += decoded_value;
            parser->ctag.toArrRef().set(s_value, myval);
          } else {
            parser->ctag.toArrRef().set(
              s_value,
              decoded_value
            );
          }
        } else {
          Array tag;
          String myval;
          String mytype;

          auto curtag = parser->data.toArrRef().pop();
          SCOPE_EXIT {
            try {
              parser->data.toArrRef().append(curtag);
            } catch (...) {}
          };

          if (curtag.toArrRef().exists(s_type)) {
            mytype = curtag.toArrRef().rvalAt(s_type).toString();
            if (!strcmp(mytype.data(), "cdata") &&
                curtag.toArrRef().exists(s_value)) {
              myval = curtag.toArrRef().rvalAt(s_value).toString();
              myval += decoded_value;
              curtag.toArrRef().set(s_value, myval);
              return;
            }
          }
          if (parser->level <= XML_MAXLEVEL) {
            tag = Array::Create();
            _xml_add_to_info(parser, parser->ltags[parser->level-1] +
                             parser->toffset);
            tag.set(s_tag, String(parser->ltags[parser->level-1] +
                                  parser->toffset, CopyString));
            tag.set(s_value, decoded_value);
            tag.set(s_type, s_cdata);
            tag.set(s_level, parser->level);
            parser->data.toArrRef().append(tag);
          } else if (parser->level == (XML_MAXLEVEL + 1)) {
            raise_warning("Maximum depth exceeded - Results truncated");
          }
        }
      }
    }
  }
}

void _xml_defaultHandler(void *userData, const XML_Char *s, int len) {
  auto parser = getParserFromToken(userData);

  if (parser && parser->defaultHandler.toBoolean()) {
    xml_call_handler(parser,
                     parser->defaultHandler,
                     make_packed_array(
                       Variant(parser),
                       _xml_xmlchar_zval(s, len, parser->target_encoding)));
  }
}

void _xml_startElementHandler(void *userData, const XML_Char *name, const XML_Char **attributes) {
  auto parser = getParserFromToken(userData);
  const char **attrs = (const char **) attributes;
  Variant retval;
  Array args = Array::Create();

  if (parser) {
    parser->level++;

    String tag_name = _xml_decode_tag(parser, (const char*)name);

    if (parser->startElementHandler.toBoolean()) {
      args.append(Variant(parser));
      args.append(tag_name);
      args.append(Array::Create());

      while (attributes && *attributes) {
        String att = _xml_decode_tag(parser, (const char*)attributes[0]);
        String val = xml_utf8_decode(attributes[1],
                                    strlen((const char*)attributes[1]),
                                    parser->target_encoding);
        args.lvalAt(2).toArrRef().set(att, val);
        attributes += 2;
      }

      xml_call_handler(parser, parser->startElementHandler, args);
    }

    if (!parser->data.isNull()) {
      if (parser->level <= XML_MAXLEVEL) {
        Array tag, atr;
        int atcnt = 0;
        tag = Array::Create();
        atr = Array::Create();

        _xml_add_to_info(parser, tag_name.substr(parser->toffset));

        tag.set(s_tag, tag_name.substr(parser->toffset));
        tag.set(s_type, s_open);
        tag.set(s_level, parser->level);

        parser->ltags[parser->level-1] = strdup(tag_name.data());
        parser->lastwasopen = 1;

        attributes = (const XML_Char **) attrs;

        while (attributes && *attributes) {
          String att = _xml_decode_tag(parser, (const char*)attributes[0]);
          String val = xml_utf8_decode(attributes[1],
                                      strlen((const char*)attributes[1]),
                                      parser->target_encoding);
          atr.set(att, val);
          atcnt++;
          attributes += 2;
        }

        if (atcnt) {
          tag.set(s_attributes,atr);
        }
        auto& lval = parser->data.toArrRef().lvalAt();
        lval.assign(tag);
        parser->ctag.assignRef(lval);
      } else if (parser->level == (XML_MAXLEVEL + 1)) {
        raise_warning("Maximum depth exceeded - Results truncated");
      }
    }
  }
}

void _xml_processingInstructionHandler(void *userData, const XML_Char *target,
                                       const XML_Char *data) {
  auto parser = getParserFromToken(userData);
  if (parser && parser->processingInstructionHandler.toBoolean()) {
    Array args = Array::Create();
    args.append(Variant(parser));
    args.append(_xml_xmlchar_zval(target, 0, parser->target_encoding));
    args.append(_xml_xmlchar_zval(data, 0, parser->target_encoding));
    xml_call_handler(parser, parser->processingInstructionHandler, args);
  }
}

int _xml_externalEntityRefHandler(XML_Parser /* void* */ parserPtr,
                                  const XML_Char *openEntityNames,
                                  const XML_Char *base,
                                  const XML_Char *systemId,
                                  const XML_Char *publicId) {
  auto parser = getParserFromToken(XML_GetUserData(parserPtr));
  int ret = 0; /* abort if no handler is set (should be configurable?) */
  if (parser && parser->externalEntityRefHandler.toBoolean()) {
    Array args = Array::Create();
    args.append(Variant(parser));
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
  auto parser = getParserFromToken(userData);

  if (parser && parser->notationDeclHandler.toBoolean()) {
    Array args = Array::Create();
    args.append(Variant(parser));
    args.append(_xml_xmlchar_zval(notationName, 0, parser->target_encoding));
    args.append(_xml_xmlchar_zval(base, 0, parser->target_encoding));
    args.append(_xml_xmlchar_zval(systemId, 0, parser->target_encoding));
    args.append(_xml_xmlchar_zval(publicId, 0, parser->target_encoding));
    xml_call_handler(parser, parser->notationDeclHandler, args);
  }
}

void _xml_startNamespaceDeclHandler(void *userData,const XML_Char *prefix,
                                    const XML_Char *uri) {
  auto parser = getParserFromToken(userData);

  if (parser && parser->startNamespaceDeclHandler.toBoolean()) {
    Array args = Array::Create();

    args.append(Variant(parser));
    args.append(_xml_xmlchar_zval(prefix, 0, parser->target_encoding));
    args.append(_xml_xmlchar_zval(uri, 0, parser->target_encoding));
    xml_call_handler(parser, parser->startNamespaceDeclHandler, args);
  }
}

void _xml_endNamespaceDeclHandler(void *userData, const XML_Char *prefix) {
  auto parser = getParserFromToken(userData);

  if (parser && parser->endNamespaceDeclHandler.toBoolean()) {
    Array args = Array::Create();
    args.append(Variant(parser));
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
  auto parser = getParserFromToken(userData);

  if (parser && parser->unparsedEntityDeclHandler.toBoolean()) {
    Array args = Array::Create();
    args.append(Variant(parser));
    args.append(_xml_xmlchar_zval(entityName, 0, parser->target_encoding));
    args.append(_xml_xmlchar_zval(base, 0, parser->target_encoding));
    args.append(_xml_xmlchar_zval(systemId, 0, parser->target_encoding));
    args.append(_xml_xmlchar_zval(publicId, 0, parser->target_encoding));
    args.append(_xml_xmlchar_zval(notationName, 0, parser->target_encoding));
    xml_call_handler(parser, parser->unparsedEntityDeclHandler, args);
  }
}

static void xml_set_handler(Variant * handler, const Variant& data) {
  if (data.isNull() || same(data, false) || data.isString() ||
      (data.isArray() && data.getArrayData()->size() == 2 &&
       (data.toCArrRef()[0].isString() || data.toCArrRef()[0].isObject()) &&
       data.toCArrRef()[1].isString())) {
    *handler = data;
  } else {
    raise_warning("Handler is invalid");
  }
}

///////////////////////////////////////////////////////////////////////////////

Resource HHVM_FUNCTION(xml_parser_create,
                       const Variant& encoding /* = null_variant */) {
  const String& strEncoding = encoding.isNull()
                            ? null_string
                            : encoding.toString();
  return php_xml_parser_create_impl(strEncoding, null_string, 0).toResource();
}

Resource HHVM_FUNCTION(xml_parser_create_ns,
                       const Variant& encoding /* = null_variant */,
                       const Variant& separator /* = null_variant */) {
  const String& strEncoding = encoding.isNull()
                            ? null_string
                            : encoding.toString();
  const String& strSeparator = separator.isNull()
                             ? null_string
                             : separator.toString();
  return php_xml_parser_create_impl(strEncoding, strSeparator, 1).toResource();
}

bool HHVM_FUNCTION(xml_parser_free,
                   const Resource& parser) {
  auto p = cast<XmlParser>(parser);
  if (p->isparsing == 1) {
    raise_warning("Parser cannot be freed while it is parsing.");
    return false;
  }
  clearParser(p);
  return true;
}

int64_t HHVM_FUNCTION(xml_parse,
                      const Resource& parser,
                      const String& data,
                      bool is_final /* = true */) {
  // XML_Parse can reenter the VM, and it will do so after we've lost
  // the frame pointer by calling through the system's copy of XML_Parse
  // in libexpat.so.
  SYNC_VM_REGS_SCOPED();
  auto p = cast<XmlParser>(parser);
  int ret;
  long isFinal = is_final ? 1 : 0;
  p->isparsing = 1;
  ret = XML_Parse(p->parser, (const XML_Char*)data.data(), data.size(),
                  isFinal);
  p->isparsing = 0;
  return ret;
}

int64_t HHVM_FUNCTION(xml_parse_into_struct,
                      const Resource& parser,
                      const String& data,
                      VRefParam values,
                      VRefParam index /* = null */) {
  SYNC_VM_REGS_SCOPED();
  int ret;
  auto p = cast<XmlParser>(parser);
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

Variant HHVM_FUNCTION(xml_parser_get_option,
                      const Resource& parser,
                      int option) {
  auto p = cast<XmlParser>(parser);
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

bool HHVM_FUNCTION(xml_parser_set_option,
                   const Resource& parser,
                   int option,
                   const Variant& value) {
  auto p = cast<XmlParser>(parser);
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

bool HHVM_FUNCTION(xml_set_character_data_handler,
                   const Resource& parser,
                   const Variant& handler) {
  auto p = cast<XmlParser>(parser);
  xml_set_handler(&p->characterDataHandler, handler);
  XML_SetCharacterDataHandler(p->parser, _xml_characterDataHandler);
  return true;
}

bool HHVM_FUNCTION(xml_set_default_handler,
                   const Resource& parser,
                   const Variant& handler) {
  auto p = cast<XmlParser>(parser);
  xml_set_handler(&p->defaultHandler, handler);
  XML_SetDefaultHandler(p->parser, _xml_defaultHandler);
  return true;
}

bool HHVM_FUNCTION(xml_set_element_handler,
                   const Resource& parser,
                   const Variant& start_element_handler,
                   const Variant& end_element_handler) {
  auto p = cast<XmlParser>(parser);
  xml_set_handler(&p->startElementHandler, start_element_handler);
  xml_set_handler(&p->endElementHandler, end_element_handler);
  XML_SetElementHandler(p->parser, _xml_startElementHandler,
                        _xml_endElementHandler);
  return true;
}

bool HHVM_FUNCTION(xml_set_processing_instruction_handler,
                   const Resource& parser,
                   const Variant& handler) {
  auto p = cast<XmlParser>(parser);
  xml_set_handler(&p->processingInstructionHandler, handler);
  XML_SetProcessingInstructionHandler(p->parser,
                                      _xml_processingInstructionHandler);
  return true;
}

bool HHVM_FUNCTION(xml_set_start_namespace_decl_handler,
                   const Resource& parser,
                   const Variant& handler) {
  auto p = cast<XmlParser>(parser);
  xml_set_handler(&p->startNamespaceDeclHandler, handler);
  XML_SetStartNamespaceDeclHandler(p->parser, _xml_startNamespaceDeclHandler);
  return true;
}

bool HHVM_FUNCTION(xml_set_end_namespace_decl_handler,
                   const Resource& parser,
                   const Variant& handler) {
  auto p = cast<XmlParser>(parser);
  xml_set_handler(&p->endNamespaceDeclHandler, handler);
  XML_SetEndNamespaceDeclHandler(p->parser, _xml_endNamespaceDeclHandler);
  return true;
}

bool HHVM_FUNCTION(xml_set_unparsed_entity_decl_handler,
                   const Resource& parser,
                   const Variant& handler) {
  auto p = cast<XmlParser>(parser);
  xml_set_handler(&p->unparsedEntityDeclHandler, handler);
  XML_SetUnparsedEntityDeclHandler(p->parser, _xml_unparsedEntityDeclHandler);
  return true;
}

bool HHVM_FUNCTION(xml_set_external_entity_ref_handler,
                   const Resource& parser,
                   const Variant& handler) {
  auto p = cast<XmlParser>(parser);
  xml_set_handler(&p->externalEntityRefHandler, handler);
  XML_SetExternalEntityRefHandler(p->parser, _xml_externalEntityRefHandler);
  return true;
}

bool HHVM_FUNCTION(xml_set_notation_decl_handler,
                   const Resource& parser,
                   const Variant& handler) {
  auto p = cast<XmlParser>(parser);
  xml_set_handler(&p->notationDeclHandler, handler);
  XML_SetNotationDeclHandler(p->parser, _xml_notationDeclHandler);
  return true;
}

bool HHVM_FUNCTION(xml_set_object,
                   const Resource& parser,
                   VRefParam object) {
  auto p = cast<XmlParser>(parser);
  p->object.assignRef(object);
  return true;
}

int64_t HHVM_FUNCTION(xml_get_current_byte_index,
                      const Resource& parser) {
  auto p = cast<XmlParser>(parser);
  return XML_GetCurrentByteIndex(p->parser);
}

int64_t HHVM_FUNCTION(xml_get_current_column_number,
                      const Resource& parser) {
  auto p = cast<XmlParser>(parser);
  return XML_GetCurrentColumnNumber(p->parser);
}

int64_t HHVM_FUNCTION(xml_get_current_line_number,
                      const Resource& parser) {
  auto p = cast<XmlParser>(parser);
  return XML_GetCurrentLineNumber(p->parser);
}

int64_t HHVM_FUNCTION(xml_get_error_code,
                      const Resource& parser) {
  auto p = cast<XmlParser>(parser);
  return XML_GetErrorCode(p->parser);
}

String HHVM_FUNCTION(xml_error_string,
                     int code) {
  char * str = (char *)XML_ErrorString((XML_Error)/*(int)*/code);
  return String(str, CopyString);
}

///////////////////////////////////////////////////////////////////////////////

String HHVM_FUNCTION(utf8_decode,
                     const String& data) {
  return xml_utf8_decode(data.c_str(), data.size(), "ISO-8859-1");
}

String HHVM_FUNCTION(utf8_encode,
                     const String& data) {
  auto const maxSize = safe_cast<size_t>(data.size()) * 4;
  String str = String(maxSize, ReserveString);
  char *newbuf = str.mutableData();
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

  assert(newlen <= maxSize);
  str.shrink(newlen);
  return str;
}

///////////////////////////////////////////////////////////////////////////////
}
