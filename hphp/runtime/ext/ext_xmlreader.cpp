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

#include "hphp/runtime/ext/ext_xmlreader.h"

#include "hphp/system/systemlib.h"

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(xmlreader);
///////////////////////////////////////////////////////////////////////////////
// constants
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
const int64_t q_XMLReader$$SIGNIFICANT_WHITESPACE = XML_READER_TYPE_SIGNIFICANT_WHITESPACE;
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

static String _xmlreader_get_valid_file_path(const char *source) {
  int isFileUri = 0;

  xmlURI *uri = xmlCreateURI();
  xmlChar *escsource = xmlURIEscapeStr((xmlChar*)source, (xmlChar*)":");
  xmlParseURIReference(uri, (char*)escsource);
  xmlFree(escsource);

  if (uri->scheme != NULL) {
    /* absolute file uris - libxml only supports localhost or empty host */
    if (strncasecmp(source, "file:///",8) == 0) {
      isFileUri = 1;
      source += 7;
    } else if (strncasecmp(source, "file://localhost/",17) == 0) {
      isFileUri = 1;
      source += 16;
    }
  }

  String file_dest = String(source, CopyString);
  if ((uri->scheme == NULL || isFileUri)) {
    file_dest = File::TranslatePath(file_dest);
  }
  xmlFreeURI(uri);
  return file_dest;
}

static xmlRelaxNGPtr _xmlreader_get_relaxNG(String source, int type,
                                            xmlRelaxNGValidityErrorFunc error_func,
                                            xmlRelaxNGValidityWarningFunc warn_func )
{
  xmlRelaxNGParserCtxtPtr parser = NULL;
  xmlRelaxNGPtr           sptr;
  String valid_file;

  switch (type) {
    case XMLREADER_LOAD_FILE:
      valid_file = _xmlreader_get_valid_file_path(source.c_str());
      if (valid_file.empty()) {
        return NULL;
      }
      parser = xmlRelaxNGNewParserCtxt(valid_file.c_str());
      break;
    case XMLREADER_LOAD_STRING:
      parser = xmlRelaxNGNewMemParserCtxt(source.data(), source.size());
      /* If loading from memory, we need to set the base directory for the document
        but it is not apparent how to do that for schema's */
      break;
    default:
      return NULL;
  }

  if (parser == NULL) {
    return NULL;
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
c_XMLReader::c_XMLReader(Class* cb) :
    ExtObjectDataFlags<ObjectData::UseGet>(cb), m_ptr(NULL), m_input(NULL), m_schema(NULL) {
}

c_XMLReader::~c_XMLReader() {
  close_impl();
}

void c_XMLReader::t___construct() {
}

bool c_XMLReader::t_open(const String& uri, const String& encoding /*= null_string*/, int64_t options /*= 0*/) {
  if (m_ptr) {
    t_close();
  }

  if (uri.empty()) {
    raise_warning("Empty string supplied as input");
    return false;
  }

  String valid_file = _xmlreader_get_valid_file_path(uri.c_str());
  xmlTextReaderPtr reader = NULL;

  if (!valid_file.empty()) {
    reader = xmlReaderForFile(valid_file.data(), encoding.data(), options);
  }

  if (reader == NULL) {
    raise_warning("Unable to open source data");
    return false;
  }

  m_ptr = reader;

  return true;
}

bool c_XMLReader::t_xml(const String& source, const String& encoding /*= null_string*/, int64_t options /*= 0*/) {
  xmlParserInputBufferPtr inputbfr = xmlParserInputBufferCreateMem(source.c_str(), source.size(), XML_CHAR_ENCODING_NONE);

  if (inputbfr != NULL) {
    char *uri = NULL;
    String directory = g_context->getCwd();
    if (!directory.empty()) {
      if (directory[directory.size() - 1] != '/') {
        directory += "/";
      }
      uri = (char *) xmlCanonicPath((const xmlChar *) directory.c_str());
    }

    xmlTextReaderPtr reader = xmlNewTextReader(inputbfr, uri);

    if (reader != NULL) {
      int ret = 0;
#if LIBXML_VERSION >= 20628
      ret = xmlTextReaderSetup(reader, NULL, uri, encoding.data(), options);
#endif
      if (ret == 0) {
        m_ptr = reader;
        m_input = inputbfr;

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

void c_XMLReader::close_impl() {
  if (m_ptr) {
    xmlFreeTextReader(m_ptr);
    m_ptr = NULL;
  }
  if (m_input) {
    xmlFreeParserInputBuffer(m_input);
    m_input = NULL;
  }
  if (m_schema) {
    xmlRelaxNGFree((xmlRelaxNGPtr) m_schema);
    m_schema = NULL;
  }
}

bool c_XMLReader::t_close() {
  close_impl();
  return true;
}

bool c_XMLReader::t_read() {
  if (m_ptr) {
    int ret = xmlTextReaderRead(m_ptr);
    if (ret == -1) {
      raise_warning("An Error Occured while reading");
      return false;
    } else {
      return ret;
    }
  }
  raise_warning("Load Data before trying to read");
  return false;
}

bool c_XMLReader::t_next(const String& localname /*= null_string*/) {
  if (m_ptr) {
    int ret = xmlTextReaderNext(m_ptr);
    while (!localname.empty() && ret == 1) {
      if (xmlStrEqual(xmlTextReaderConstLocalName(m_ptr), (xmlChar *)localname.data())) {
        return true;
      }
      ret = xmlTextReaderNext(m_ptr);
    }
    if (ret == -1) {
      raise_warning("An Error Occured while reading");
      return false;
    } else {
      return ret;
    }
  }
  raise_warning("Load Data before trying to read");
  return false;
}

String c_XMLReader::read_string_func(xmlreader_read_char_t internal_function) {
  char *retchar = NULL;
  if (m_ptr) {
    retchar = (char *)internal_function(m_ptr);
  }
  if (retchar) {
    String ret((const char*)retchar, CopyString);
    xmlFree(retchar);
    return ret;
  } else {
    return String("");
  }
}

String c_XMLReader::t_readstring() {
  return read_string_func(xmlTextReaderReadString);
}

String c_XMLReader::t_readinnerxml() {
  return read_string_func(xmlTextReaderReadInnerXml);
}

String c_XMLReader::t_readouterxml() {
  return read_string_func(xmlTextReaderReadString);
}

bool c_XMLReader::bool_func_no_arg(xmlreader_read_int_t internal_function) {
  if (m_ptr) {
    int ret = internal_function(m_ptr);
    if (ret == 1) {
      return true;
    }
  }
  return false;
}

Variant c_XMLReader::string_func_string_arg(String value, xmlreader_read_one_char_t internal_function) {

  if (value.empty()) {
    raise_warning("Argument cannot be an empty string");
    return false;
  }

  char *retchar = NULL;
  if (m_ptr) {
    retchar = (char *)internal_function(m_ptr, (const unsigned char *)value.data());
  }

  if (retchar) {
    String ret((const char*)retchar, CopyString);
    xmlFree(retchar);
    return ret;
  } else {
    return uninit_null();
  }
}

Variant c_XMLReader::t_getattribute(const String& name) {
  return string_func_string_arg(name, xmlTextReaderGetAttribute);
}

Variant c_XMLReader::t_getattributeno(int64_t index) {
  char *retchar = NULL;
  if (m_ptr) {
    retchar = (char *)xmlTextReaderGetAttributeNo(m_ptr, index);
  }
  if (retchar) {
    String ret((const char*)retchar, CopyString);
    xmlFree(retchar);
    return ret;
  } else {
    return uninit_null();
  }
}

Variant c_XMLReader::t_getattributens(const String& name, const String& namespaceURI) {
  if (name.empty() || namespaceURI.empty()) {
    raise_warning("Attribute Name and Namespace URI cannot be empty");
    return false;
  }

  char *retchar = NULL;
  if (m_ptr) {
    retchar = (char *)xmlTextReaderGetAttributeNs(m_ptr,
                                                  (xmlChar *)name.data(),
                                                  (xmlChar *)namespaceURI.data());
  }

  if (retchar) {
    String ret((const char*)retchar, CopyString);
    xmlFree(retchar);
    return ret;
  } else {
    return uninit_null();
  }
}

bool c_XMLReader::t_movetoattribute(const String& name) {
  if (name.empty()) {
    raise_warning("Attribute Name is required");
    return false;
  }

  if (m_ptr) {
    int ret = xmlTextReaderMoveToAttribute(m_ptr, (xmlChar *)name.data());
    if (ret == 1) {
      return true;
    }
  }
  return false;
}

bool c_XMLReader::t_movetoattributeno(int64_t index) {
  if (m_ptr) {
    int ret = xmlTextReaderMoveToAttributeNo(m_ptr, index);
    if (ret == 1) {
      return true;
    }
  }
  return false;
}

bool c_XMLReader::t_movetoattributens(const String& name, const String& namespaceURI) {
  if (name.empty() || namespaceURI.empty()) {
    raise_warning("Attribute Name and Namespace URI cannot be empty");
    return false;
  }
  if (m_ptr) {
    int ret = xmlTextReaderMoveToAttributeNs(m_ptr,
                                             (xmlChar *)name.data(),
                                             (xmlChar *)namespaceURI.data());
    if (ret == 1) {
      return true;
    }
  }
  return false;
}

bool c_XMLReader::t_movetoelement() {
  return bool_func_no_arg(xmlTextReaderMoveToElement);
}

bool c_XMLReader::t_movetofirstattribute() {
  return bool_func_no_arg(xmlTextReaderMoveToFirstAttribute);
}

bool c_XMLReader::t_movetonextattribute() {
  return bool_func_no_arg(xmlTextReaderMoveToNextAttribute);
}

bool c_XMLReader::t_isvalid() {
  return bool_func_no_arg(xmlTextReaderIsValid);
}

bool c_XMLReader::t_getparserproperty(int64_t property) {
  int ret = 0;
  if (m_ptr) {
    ret = xmlTextReaderGetParserProp(m_ptr, property);
  }
  if (ret == -1) {
    raise_warning("Invalid parser property");
    return false;
  }
  return ret;
}

Variant c_XMLReader::t_lookupnamespace(const String& prefix) {
  return string_func_string_arg(prefix, xmlTextReaderLookupNamespace);
}

bool c_XMLReader::t_setschema(const String& source) {
  if (source.empty()) {
    raise_warning("Schema data source is required");
    return false;
  }

  if (m_ptr) {
    int ret = xmlTextReaderSchemaValidate(m_ptr, source.c_str());
    if (ret == 0) {
      return true;
    }
  }
  raise_warning("Unable to set schema. This must be set prior to reading or schema contains errors.");
  return false;
}

bool c_XMLReader::t_setparserproperty(int64_t property, bool value) {
  if (m_ptr) {
    int ret = xmlTextReaderSetParserProp(m_ptr, property, value);
    if (ret == -1) {
      raise_warning("Invalid parser property");
      return false;
    }
    return true;
  }
  return false;
}

bool c_XMLReader::set_relaxng_schema(String source, int type) {
  if (source.empty()) {
    raise_warning("Schema data source is required");
    return false;
  }

  if (m_ptr) {
    int ret = -1;
    xmlRelaxNGPtr schema = NULL;
    if (!source.empty()) {
      schema =  _xmlreader_get_relaxNG(source, type, NULL, NULL);
      if (schema) {
        ret = xmlTextReaderRelaxNGSetSchema(m_ptr, schema);
      }
    } else {
      ret = xmlTextReaderRelaxNGSetSchema(m_ptr, NULL);
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

bool c_XMLReader::t_setrelaxngschema(const String& filename) {
  return set_relaxng_schema(filename, XMLREADER_LOAD_FILE);
}

bool c_XMLReader::t_setrelaxngschemasource(const String& source) {
  return set_relaxng_schema(source, XMLREADER_LOAD_STRING);
}

///////////////////////////////////////////////////////////////////////////////

struct PropertyAccessor {
  const char *name;
  int (*getter_int)(xmlTextReaderPtr);
  const xmlChar* (*getter_char)(xmlTextReaderPtr);
  int return_type;
};

class PropertyAccessorMap : private hphp_const_char_imap<PropertyAccessor*> {
public:
  explicit PropertyAccessorMap(PropertyAccessor* props,
                               PropertyAccessorMap *base = nullptr) {
    if (base) {
      *this = *base;
    }
    for (PropertyAccessor *p = props; p->name; p++) {
      (*this)[p->name] = p;
    }
  }

  PropertyAccessor* get(CVarRef name) {
    if (name.isString()) {
      const_iterator iter = find(name.toString().data());
      if (iter != end()) {
        return iter->second;
      }
    }
    return NULL;
  }
};

static PropertyAccessor xmlreader_properties[] = {
  { "attributeCount", xmlTextReaderAttributeCount, NULL, KindOfInt64 },
  { "baseURI", NULL, xmlTextReaderConstBaseUri, KindOfString },
  { "depth", xmlTextReaderDepth, NULL, KindOfInt64 },
  { "hasAttributes", xmlTextReaderHasAttributes, NULL, KindOfBoolean },
  { "hasValue", xmlTextReaderHasValue, NULL, KindOfBoolean },
  { "isDefault", xmlTextReaderIsDefault, NULL, KindOfBoolean },
  { "isEmptyElement", xmlTextReaderIsEmptyElement, NULL, KindOfBoolean },
  { "localName", NULL, xmlTextReaderConstLocalName, KindOfString },
  { "name", NULL, xmlTextReaderConstName, KindOfString },
  { "namespaceURI", NULL, xmlTextReaderConstNamespaceUri, KindOfString },
  { "nodeType", xmlTextReaderNodeType, NULL, KindOfInt64 },
  { "prefix", NULL, xmlTextReaderConstPrefix, KindOfString },
  { "value", NULL, xmlTextReaderConstValue, KindOfString },
  { "xmlLang", NULL, xmlTextReaderConstXmlLang, KindOfString },
  { NULL, NULL, NULL }
};

static PropertyAccessorMap xmlreader_properties_map
((PropertyAccessor*)xmlreader_properties);

Variant c_XMLReader::t___get(Variant name) {
  const xmlChar *retchar = NULL;
  int retint = 0;

  PropertyAccessor *propertyMap = xmlreader_properties_map.get(name);
  if (!propertyMap) {
    raiseUndefProp(name.getStringData());
    return uninit_null();
  }

  if (m_ptr) {
    if (propertyMap->getter_char) {
      retchar = propertyMap->getter_char(m_ptr);
    } else if (propertyMap->getter_int) {
      retint = propertyMap->getter_int(m_ptr);
    }
  }

  switch (propertyMap->return_type) {
    case KindOfString:
      if (retchar) {
        return String((char*)retchar, CopyString);
      } else {
        return String("");
      }
    case KindOfBoolean:
      return (retint ? true : false);

    case KindOfInt64:
      return retint;

    default:
      return uninit_null();
  }
  return uninit_null();
}

Variant c_XMLReader::t___destruct() {
  return uninit_null();
}

///////////////////////////////////////////////////////////////////////////////
}
