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

#include "hphp/runtime/ext/xmlreader/ext_xmlreader.h"
#include "hphp/runtime/ext/domdocument/ext_domdocument.h"
#include "hphp/runtime/ext/libxml/ext_libxml.h"

#include "hphp/util/functional.h"
#include "hphp/util/hash-map-typedefs.h"
#include "hphp/system/systemlib.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// constants

const StaticString s_XMLReader("XMLReader");

const int64_t q_XMLReader$$NONE = XML_READER_TYPE_NONE;
const int64_t q_XMLReader$$ELEMENT = XML_READER_TYPE_ELEMENT;
const int64_t q_XMLReader$$ATTRIBUTE = XML_READER_TYPE_ATTRIBUTE;
const int64_t q_XMLReader$$TEXT = XML_READER_TYPE_TEXT;
const int64_t q_XMLReader$$CDATA = XML_READER_TYPE_CDATA;
const int64_t q_XMLReader$$ENTITY_REF = XML_READER_TYPE_ENTITY_REFERENCE;
const int64_t q_XMLReader$$ENTITY = XML_READER_TYPE_ENTITY;
const int64_t q_XMLReader$$PI = XML_READER_TYPE_PROCESSING_INSTRUCTION;
const int64_t q_XMLReader$$COMMENT = XML_READER_TYPE_COMMENT;
const int64_t q_XMLReader$$DOC = XML_READER_TYPE_DOCUMENT;
const int64_t q_XMLReader$$DOC_TYPE = XML_READER_TYPE_DOCUMENT_TYPE;
const int64_t q_XMLReader$$DOC_FRAGMENT = XML_READER_TYPE_DOCUMENT_FRAGMENT;
const int64_t q_XMLReader$$NOTATION = XML_READER_TYPE_NOTATION;
const int64_t q_XMLReader$$WHITESPACE = XML_READER_TYPE_WHITESPACE;
const int64_t q_XMLReader$$SIGNIFICANT_WHITESPACE =
  XML_READER_TYPE_SIGNIFICANT_WHITESPACE;
const int64_t q_XMLReader$$END_ELEMENT = XML_READER_TYPE_END_ELEMENT;
const int64_t q_XMLReader$$END_ENTITY = XML_READER_TYPE_END_ENTITY;
const int64_t q_XMLReader$$XML_DECLARATION = XML_READER_TYPE_XML_DECLARATION;
const int64_t q_XMLReader$$LOADDTD = XML_PARSER_LOADDTD;
const int64_t q_XMLReader$$DEFAULTATTRS = XML_PARSER_DEFAULTATTRS;
const int64_t q_XMLReader$$VALIDATE = XML_PARSER_VALIDATE;
const int64_t q_XMLReader$$SUBST_ENTITIES = XML_PARSER_SUBST_ENTITIES;

#define XMLREADER_LOAD_STRING 0
#define XMLREADER_LOAD_FILE 1

///////////////////////////////////////////////////////////////////////////////
// helpers

static xmlRelaxNGPtr _xmlreader_get_relaxNG(String source, int type,
    xmlRelaxNGValidityErrorFunc error_func,
    xmlRelaxNGValidityWarningFunc warn_func) {
  xmlRelaxNGParserCtxtPtr parser = nullptr;
  xmlRelaxNGPtr           sptr;
  String valid_file;

  switch (type) {
    case XMLREADER_LOAD_FILE:
      valid_file = libxml_get_valid_file_path(source.c_str());
      if (valid_file.empty()) {
        return nullptr;
      }
      parser = xmlRelaxNGNewParserCtxt(valid_file.c_str());
      break;
    case XMLREADER_LOAD_STRING:
      parser = xmlRelaxNGNewMemParserCtxt(source.data(), source.size());
      // If loading from memory, we need to set the base directory for the
      // document but it is not apparent how to do that for schema's
      break;
    default:
      return nullptr;
  }

  if (parser == nullptr) {
    return nullptr;
  }

  if (error_func || warn_func) {
  xmlRelaxNGSetParserErrors(parser,
    (xmlRelaxNGValidityErrorFunc) error_func,
    (xmlRelaxNGValidityWarningFunc) warn_func,
    parser);
  }
  sptr = xmlRelaxNGParse(parser);
  xmlRelaxNGFreeParserCtxt(parser);

  return sptr;
}

///////////////////////////////////////////////////////////////////////////////
XMLReader::XMLReader() : m_ptr(nullptr), m_input(nullptr), m_schema(nullptr) {
}

XMLReader::~XMLReader() {
  close();
}

void XMLReader::close() {
  SYNC_VM_REGS_SCOPED();
  if (m_stream) {
    m_stream->close();
    m_stream = nullptr;
  }
  if (m_ptr) {
    xmlFreeTextReader(m_ptr);
    m_ptr = nullptr;
  }
  if (m_input) {
    xmlFreeParserInputBuffer(m_input);
    m_input = nullptr;
  }
  if (m_schema) {
    xmlRelaxNGFree((xmlRelaxNGPtr) m_schema);
    m_schema = nullptr;
  }
}

bool HHVM_METHOD(XMLReader, open,
                 const String& uri,
                 const Variant& encoding /*= null_variant*/,
                 int64_t options /*= 0*/) {
  auto* data = Native::data<XMLReader>(this_);
  const String& str_encoding = encoding.isNull()
                            ? null_string
                            : encoding.toString();
  SYNC_VM_REGS_SCOPED();
  if (data->m_ptr) {
    data->close();
  }

  if (uri.empty()) {
    raise_warning("Empty string supplied as input");
    return false;
  }

  String valid_file = libxml_get_valid_file_path(uri.c_str());
  xmlTextReaderPtr reader = nullptr;

  if (!valid_file.empty()) {
    // Manually create the IO context to support custom stream wrappers.
    data->m_stream = File::Open(valid_file, "rb");
    if (data->m_stream != nullptr && !data->m_stream->isInvalid()) {
      // The XML context is owned by the native data attached to 'this_'.
      // The File is also owned by the native data so it does not need
      // to be cleaned up by an XML callback.  The libxml_streams_IO_nop_close
      // callback does nothing.
      reader = xmlReaderForIO(libxml_streams_IO_read,
                              libxml_streams_IO_nop_close,
                              &data->m_stream,
                              valid_file.data(),
                              str_encoding.data(),
                              options);
    }
  }

  if (reader == nullptr) {
    raise_warning("Unable to open source data");
    return false;
  }

  data->m_ptr = reader;

  return true;
}

bool HHVM_METHOD(XMLReader, XML,
                 const String& source,
                 const Variant& encoding /*= null_variant*/,
                 int64_t options /*= 0*/) {
  auto* data = Native::data<XMLReader>(this_);
  const String& str_encoding = encoding.isNull()
                            ? null_string
                            : encoding.toString();
  xmlParserInputBufferPtr inputbfr = xmlParserInputBufferCreateMem(
    source.c_str(), source.size(), XML_CHAR_ENCODING_NONE);

  if (inputbfr != nullptr) {
    char *uri = nullptr;
    String directory = g_context->getCwd();
    if (!directory.empty()) {
      if (directory[directory.size() - 1] != '/') {
        directory += "/";
      }
      uri = (char *) xmlCanonicPath((const xmlChar *) directory.c_str());
    }

    xmlTextReaderPtr reader = xmlNewTextReader(inputbfr, uri);

    if (reader != nullptr) {
      int ret = 0;
#if LIBXML_VERSION >= 20628
      ret = xmlTextReaderSetup(reader, nullptr, uri, str_encoding.data(),
                               options);
#endif
      if (ret == 0) {
        data->m_ptr = reader;
        data->m_input = inputbfr;

        if (uri) {
          xmlFree(uri);
        }

        return true;
      }
    }

    if (uri) {
      xmlFree(uri);
    }
  }

  if (inputbfr) {
    xmlFreeParserInputBuffer(inputbfr);
  }

  raise_warning("Unable to load source data");
  return false;
}

bool HHVM_METHOD(XMLReader, close) {
  auto* data = Native::data<XMLReader>(this_);
  data->close();
  return true;
}

bool HHVM_METHOD(XMLReader, read) {
  auto* data = Native::data<XMLReader>(this_);
  SYNC_VM_REGS_SCOPED();
  if (data->m_ptr) {
    int ret = xmlTextReaderRead(data->m_ptr);
    if (ret == -1) {
      raise_warning("An Error Occurred while reading");
      return false;
    } else {
      return ret;
    }
  }
  raise_warning("Load Data before trying to read");
  return false;
}

bool HHVM_METHOD(XMLReader, next,
                 const Variant& localname /*= null_variant*/) {
  auto* data = Native::data<XMLReader>(this_);
  const String& str_localname = localname.isNull()
                              ? null_string
                              : localname.toString();
  SYNC_VM_REGS_SCOPED();
  if (data->m_ptr) {
    int ret = xmlTextReaderNext(data->m_ptr);
    while (!str_localname.empty() && ret == 1) {
      if (xmlStrEqual(xmlTextReaderConstLocalName(data->m_ptr),
          (xmlChar *)str_localname.data())) {
        return true;
      }
      ret = xmlTextReaderNext(data->m_ptr);
    }
    if (ret == -1) {
      raise_warning("An Error Occurred while reading");
      return false;
    } else {
      return ret;
    }
  }
  raise_warning("Load Data before trying to read");
  return false;
}

String XMLReader::read_string_func(xmlreader_read_char_t internal_function) {
  SYNC_VM_REGS_SCOPED();
  char *retchar = nullptr;
  if (m_ptr) {
    retchar = (char *)internal_function(m_ptr);
  }
  if (retchar) {
    String ret((const char*)retchar, CopyString);
    xmlFree(retchar);
    return ret;
  } else {
    return empty_string();
  }
}

String HHVM_METHOD(XMLReader, readString) {
  auto* data = Native::data<XMLReader>(this_);
  return data->read_string_func(xmlTextReaderReadString);
}

String HHVM_METHOD(XMLReader, readInnerXML) {
  auto* data = Native::data<XMLReader>(this_);
  return data->read_string_func(xmlTextReaderReadInnerXml);
}

String HHVM_METHOD(XMLReader, readOuterXML) {
  auto* data = Native::data<XMLReader>(this_);
  return data->read_string_func(xmlTextReaderReadOuterXml);
}

bool XMLReader::bool_func_no_arg(xmlreader_read_int_t internal_function) {
  SYNC_VM_REGS_SCOPED();
  if (m_ptr) {
    int ret = internal_function(m_ptr);
    if (ret == 1) {
      return true;
    }
  }
  return false;
}

Variant XMLReader::string_func_string_arg(String value,
    xmlreader_read_one_char_t internal_function) {
  SYNC_VM_REGS_SCOPED();

  if (value.empty()) {
    raise_warning("Argument cannot be an empty string");
    return false;
  }

  char *retchar = nullptr;
  if (m_ptr) {
    retchar = (char *)internal_function(m_ptr, (const unsigned char *)value.data());
  }

  if (retchar) {
    String ret((const char*)retchar, CopyString);
    xmlFree(retchar);
    return ret;
  } else {
    return init_null();
  }
}

Variant HHVM_METHOD(XMLReader, getAttribute,
                    const String& name) {
  auto* data = Native::data<XMLReader>(this_);
  return data->string_func_string_arg(name, xmlTextReaderGetAttribute);
}

Variant HHVM_METHOD(XMLReader, getAttributeNo,
                    int64_t index) {
  auto* data = Native::data<XMLReader>(this_);
  SYNC_VM_REGS_SCOPED();
  char *retchar = nullptr;
  if (data->m_ptr) {
    retchar = (char *)xmlTextReaderGetAttributeNo(data->m_ptr, index);
  }
  if (retchar) {
    String ret((const char*)retchar, CopyString);
    xmlFree(retchar);
    return ret;
  } else {
    return init_null();
  }
}

Variant HHVM_METHOD(XMLReader, getAttributeNs,
                    const String& name,
                    const String& namespaceURI) {
  auto* data = Native::data<XMLReader>(this_);
  SYNC_VM_REGS_SCOPED();
  if (name.empty() || namespaceURI.empty()) {
    raise_warning("Attribute Name and Namespace URI cannot be empty");
    return false;
  }

  char *retchar = nullptr;
  if (data->m_ptr) {
    retchar = (char *)xmlTextReaderGetAttributeNs(data->m_ptr,
                                                  (xmlChar *)name.data(),
                                                  (xmlChar *)namespaceURI.data());
  }

  if (retchar) {
    String ret((const char*)retchar, CopyString);
    xmlFree(retchar);
    return ret;
  } else {
    return init_null();
  }
}

bool HHVM_METHOD(XMLReader, moveToAttribute,
                 const String& name) {
  auto* data = Native::data<XMLReader>(this_);
  SYNC_VM_REGS_SCOPED();
  if (name.empty()) {
    raise_warning("Attribute Name is required");
    return false;
  }

  if (data->m_ptr) {
    int ret = xmlTextReaderMoveToAttribute(data->m_ptr, (xmlChar *)name.data());
    if (ret == 1) {
      return true;
    }
  }
  return false;
}

bool HHVM_METHOD(XMLReader, moveToAttributeNo,
                 int64_t index) {
  auto* data = Native::data<XMLReader>(this_);
  SYNC_VM_REGS_SCOPED();
  if (data->m_ptr) {
    int ret = xmlTextReaderMoveToAttributeNo(data->m_ptr, index);
    if (ret == 1) {
      return true;
    }
  }
  return false;
}

bool HHVM_METHOD(XMLReader, moveToAttributeNs,
                 const String& name,
                 const String& namespaceURI) {
  auto* data = Native::data<XMLReader>(this_);
  SYNC_VM_REGS_SCOPED();
  if (name.empty() || namespaceURI.empty()) {
    raise_warning("Attribute Name and Namespace URI cannot be empty");
    return false;
  }
  if (data->m_ptr) {
    int ret = xmlTextReaderMoveToAttributeNs(data->m_ptr,
                                             (xmlChar *)name.data(),
                                             (xmlChar *)namespaceURI.data());
    if (ret == 1) {
      return true;
    }
  }
  return false;
}

bool HHVM_METHOD(XMLReader, moveToElement) {
  auto* data = Native::data<XMLReader>(this_);
  return data->bool_func_no_arg(xmlTextReaderMoveToElement);
}

bool HHVM_METHOD(XMLReader, moveToFirstAttribute) {
  auto* data = Native::data<XMLReader>(this_);
  return data->bool_func_no_arg(xmlTextReaderMoveToFirstAttribute);
}

bool HHVM_METHOD(XMLReader, moveToNextAttribute) {
  auto* data = Native::data<XMLReader>(this_);
  return data->bool_func_no_arg(xmlTextReaderMoveToNextAttribute);
}

bool HHVM_METHOD(XMLReader, isValid) {
  auto* data = Native::data<XMLReader>(this_);
  return data->bool_func_no_arg(xmlTextReaderIsValid);
}

bool HHVM_METHOD(XMLReader, getParserProperty,
                 int64_t property) {
  auto* data = Native::data<XMLReader>(this_);
  int ret = 0;
  if (data->m_ptr) {
    ret = xmlTextReaderGetParserProp(data->m_ptr, property);
  }
  if (ret == -1) {
    raise_warning("Invalid parser property");
    return false;
  }
  return ret;
}

Variant HHVM_METHOD(XMLReader, lookupNamespace,
                    const String& prefix) {
  auto* data = Native::data<XMLReader>(this_);
  return data->string_func_string_arg(prefix, xmlTextReaderLookupNamespace);
}

bool HHVM_METHOD(XMLReader, setSchema,
                 const String& source) {
  auto* data = Native::data<XMLReader>(this_);
  SYNC_VM_REGS_SCOPED();
  if (source.empty()) {
    raise_warning("Schema data source is required");
    return false;
  }

  if (data->m_ptr) {
    int ret = xmlTextReaderSchemaValidate(data->m_ptr, source.c_str());
    if (ret == 0) {
      return true;
    }
  }
  raise_warning("Unable to set schema. This must be set prior to reading or schema contains errors.");
  return false;
}

bool HHVM_METHOD(XMLReader, setParserProperty,
                 int64_t property,
                 bool value) {
  auto* data = Native::data<XMLReader>(this_);
  if (data->m_ptr) {
    int ret = xmlTextReaderSetParserProp(data->m_ptr, property, value);
    if (ret == -1) {
      raise_warning("Invalid parser property");
      return false;
    }
    return true;
  }
  return false;
}

bool XMLReader::set_relaxng_schema(String source, int type) {
  SYNC_VM_REGS_SCOPED();
  if (source.empty()) {
    raise_warning("Schema data source is required");
    return false;
  }

  if (m_ptr) {
    int ret = -1;
    xmlRelaxNGPtr schema = nullptr;
    if (!source.empty()) {
      schema =  _xmlreader_get_relaxNG(source, type, nullptr, nullptr);
      if (schema) {
        ret = xmlTextReaderRelaxNGSetSchema(m_ptr, schema);
      }
    } else {
      ret = xmlTextReaderRelaxNGSetSchema(m_ptr, nullptr);
    }

    if (ret == 0) {
      if (m_schema) {
        xmlRelaxNGFree((xmlRelaxNGPtr) m_schema);
      }
      m_schema = schema;
      return true;
    }
  }

  raise_warning("Unable to set schema. This must be set prior to reading or schema contains errors.");
  return false;
}

bool HHVM_METHOD(XMLReader, setRelaxNGSchema,
                 const String& filename) {
  auto* data = Native::data<XMLReader>(this_);
  return data->set_relaxng_schema(filename, XMLREADER_LOAD_FILE);
}

bool HHVM_METHOD(XMLReader, setRelaxNGSchemaSource,
                 const String& source) {
  auto* data = Native::data<XMLReader>(this_);
  return data->set_relaxng_schema(source, XMLREADER_LOAD_STRING);
}

///////////////////////////////////////////////////////////////////////////////

struct XMLPropertyAccessor {
  const char *name;
  int (*getter_int)(xmlTextReaderPtr);
  const xmlChar* (*getter_char)(xmlTextReaderPtr);
  int return_type;
};

class XMLPropertyAccessorMap :
      private hphp_const_char_map<XMLPropertyAccessor*> {
public:
  explicit XMLPropertyAccessorMap(XMLPropertyAccessor* props,
                                  XMLPropertyAccessorMap *base = nullptr) {
    if (base) {
      *this = *base;
    }
    for (XMLPropertyAccessor *p = props; p->name; p++) {
      (*this)[p->name] = p;
      m_imap[p->name] = p;
    }
  }

  XMLPropertyAccessor* get(const Variant& name) {
    if (name.isString()) {
      const char* name_data = name.toString().data();
      const_iterator iter = find(name_data);
      const_iterator iiter = m_imap.find(name_data);
      if (iter != end()) {
        return iter->second;
      } else if (iiter != end()) {
        raise_warning("Accessing XMLReader::$%s with the incorrect casing",
                      name_data);
        return iiter->second;
      }
    }
    return nullptr;
  }
private:
  // Previously, this class was backed by an imap. This led to a lot of
  // code relying on accessing properties that were improperly cased.
  // Since removing this functionality could cause a lot of functionality
  // to break, instead we continue to allow access case-insensitively, but
  // with a warning
  hphp_const_char_imap<XMLPropertyAccessor*> m_imap;
};

static XMLPropertyAccessor xmlreader_properties[] = {
  { "attributeCount", xmlTextReaderAttributeCount, nullptr, KindOfInt64 },
  { "baseURI", nullptr, xmlTextReaderConstBaseUri, KindOfString },
  { "depth", xmlTextReaderDepth, nullptr, KindOfInt64 },
  { "hasAttributes", xmlTextReaderHasAttributes, nullptr, KindOfBoolean },
  { "hasValue", xmlTextReaderHasValue, nullptr, KindOfBoolean },
  { "isDefault", xmlTextReaderIsDefault, nullptr, KindOfBoolean },
  { "isEmptyElement", xmlTextReaderIsEmptyElement, nullptr, KindOfBoolean },
  { "localName", nullptr, xmlTextReaderConstLocalName, KindOfString },
  { "name", nullptr, xmlTextReaderConstName, KindOfString },
  { "namespaceURI", nullptr, xmlTextReaderConstNamespaceUri, KindOfString },
  { "nodeType", xmlTextReaderNodeType, nullptr, KindOfInt64 },
  { "prefix", nullptr, xmlTextReaderConstPrefix, KindOfString },
  { "value", nullptr, xmlTextReaderConstValue, KindOfString },
  { "xmlLang", nullptr, xmlTextReaderConstXmlLang, KindOfString },
  { nullptr, nullptr, nullptr }
};

static XMLPropertyAccessorMap xmlreader_properties_map
((XMLPropertyAccessor*)xmlreader_properties);

Variant HHVM_METHOD(XMLReader, __get,
                    const Variant& name) {
  auto* data = Native::data<XMLReader>(this_);
  const xmlChar *retchar = nullptr;
  int retint = 0;

  XMLPropertyAccessor *propertyMap = xmlreader_properties_map.get(name);
  if (!propertyMap) {
    this_->raiseUndefProp(name.getStringData());
    return init_null();
  }

  if (data->m_ptr) {
    if (propertyMap->getter_char) {
      retchar = propertyMap->getter_char(data->m_ptr);
    } else if (propertyMap->getter_int) {
      retint = propertyMap->getter_int(data->m_ptr);
    }
  }

  switch (propertyMap->return_type) {
    case KindOfBoolean:
      return retint ? true : false;
    case KindOfInt64:
      return retint;
    case KindOfString:
      if (retchar) {
        return String((char*)retchar, CopyString);
      } else {
        return empty_string_variant();
      }
    case KindOfUninit:
    case KindOfNull:
    case KindOfDouble:
    case KindOfStaticString:
    case KindOfArray:
    case KindOfObject:
    case KindOfResource:
    case KindOfRef:
      return init_null();

    case KindOfClass:
      break;
  }
  not_reached();
}

Variant HHVM_METHOD(XMLReader, expand,
                    const Variant& basenode /* = null */) {
  auto* data = Native::data<XMLReader>(this_);
  req::ptr<XMLDocumentData> doc;
  xmlDocPtr docp = nullptr;
  SYNC_VM_REGS_SCOPED();

  if (!basenode.isNull()) {
    auto dombasenode = Native::data<DOMNode>(basenode.toObject());
    doc = dombasenode->doc();
    docp = doc->docp();
    if (docp == nullptr) {
      raise_warning("Invalid State Error");
      return false;
    }
  }

  if (data->m_ptr) {
    xmlNodePtr node = xmlTextReaderExpand(data->m_ptr);
    if (node == nullptr) {
      raise_warning("An Error Occurred while expanding");
      return false;
    } else {
      xmlNodePtr nodec = xmlDocCopyNode(node, docp, 1);
      if (nodec == nullptr) {
        raise_notice("Cannot expand this node type");
        return false;
      } else {
        return php_dom_create_object(nodec, doc);
      }
    }
  }

  raise_warning("Load Data before trying to read");
  return false;
}

///////////////////////////////////////////////////////////////////////////////

#define REGISTER_XML_READER_CONSTANT(name)                                     \
  Native::registerClassConstant<KindOfInt64>(s_XMLReader.get(),                \
                                             makeStaticString(#name),          \
                                             q_XMLReader$$##name)              \

static class XMLReaderExtension final : public Extension {
public:
  XMLReaderExtension() : Extension("xmlreader", "0.1") {}
  void moduleInit() override {
    REGISTER_XML_READER_CONSTANT(NONE);
    REGISTER_XML_READER_CONSTANT(ELEMENT);
    REGISTER_XML_READER_CONSTANT(ATTRIBUTE);
    REGISTER_XML_READER_CONSTANT(TEXT);
    REGISTER_XML_READER_CONSTANT(CDATA);
    REGISTER_XML_READER_CONSTANT(ENTITY_REF);
    REGISTER_XML_READER_CONSTANT(ENTITY);
    REGISTER_XML_READER_CONSTANT(PI);
    REGISTER_XML_READER_CONSTANT(COMMENT);
    REGISTER_XML_READER_CONSTANT(DOC);
    REGISTER_XML_READER_CONSTANT(DOC_TYPE);
    REGISTER_XML_READER_CONSTANT(DOC_FRAGMENT);
    REGISTER_XML_READER_CONSTANT(NOTATION);
    REGISTER_XML_READER_CONSTANT(WHITESPACE);
    REGISTER_XML_READER_CONSTANT(SIGNIFICANT_WHITESPACE);
    REGISTER_XML_READER_CONSTANT(END_ELEMENT);
    REGISTER_XML_READER_CONSTANT(END_ENTITY);
    REGISTER_XML_READER_CONSTANT(XML_DECLARATION);
    REGISTER_XML_READER_CONSTANT(LOADDTD);
    REGISTER_XML_READER_CONSTANT(DEFAULTATTRS);
    REGISTER_XML_READER_CONSTANT(VALIDATE);
    REGISTER_XML_READER_CONSTANT(SUBST_ENTITIES);

    HHVM_ME(XMLReader, open);
    HHVM_ME(XMLReader, XML);
    HHVM_ME(XMLReader, close);
    HHVM_ME(XMLReader, read);
    HHVM_ME(XMLReader, next);
    HHVM_ME(XMLReader, readString);
    HHVM_ME(XMLReader, readInnerXML);
    HHVM_ME(XMLReader, readOuterXML);
    HHVM_ME(XMLReader, moveToNextAttribute);
    HHVM_ME(XMLReader, getAttribute);
    HHVM_ME(XMLReader, getAttributeNo);
    HHVM_ME(XMLReader, getAttributeNs);
    HHVM_ME(XMLReader, moveToAttribute);
    HHVM_ME(XMLReader, moveToAttributeNo);
    HHVM_ME(XMLReader, moveToAttributeNs);
    HHVM_ME(XMLReader, moveToElement);
    HHVM_ME(XMLReader, moveToFirstAttribute);
    HHVM_ME(XMLReader, isValid);
    HHVM_ME(XMLReader, __get);
    HHVM_ME(XMLReader, getParserProperty);
    HHVM_ME(XMLReader, lookupNamespace);
    HHVM_ME(XMLReader, setSchema);
    HHVM_ME(XMLReader, setParserProperty);
    HHVM_ME(XMLReader, setRelaxNGSchema);
    HHVM_ME(XMLReader, setRelaxNGSchemaSource);
    HHVM_ME(XMLReader, expand);

    Native::registerNativeDataInfo<XMLReader>(s_XMLReader.get());

    loadSystemlib();
  }
} s_xml_reader_extension;

///////////////////////////////////////////////////////////////////////////////
}
