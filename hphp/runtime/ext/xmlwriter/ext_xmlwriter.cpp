/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/vm/vm-regs.h"

#include <libxml/tree.h>
#include <libxml/uri.h>
#include <libxml/xmlwriter.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
const StaticString s_XMLWriter("XMLWriter");

struct XMLWriterData {
  XMLWriterData(): m_ptr(nullptr), m_output(nullptr) {}

  ~XMLWriterData() {
    sweep();
  }

  void sweep() {
    if (m_ptr) {
      xmlFreeTextWriter(m_ptr);
      m_ptr = nullptr;
    }
    if (m_output) {
      xmlBufferFree(m_output);
      m_output = nullptr;
    }
  }

  bool openMemory() {
    m_output = xmlBufferCreate();
    if (m_output == nullptr) {
      raise_warning("Unable to create output buffer");
      return false;
    }
    return (m_ptr = xmlNewTextWriterMemory(m_output, 0));
  }

  bool openURI(const String& uri) {
    m_uri = File::Open(uri, "wb");
    if (!m_uri) {
      return false;
    }

    xmlOutputBufferPtr uri_output = xmlOutputBufferCreateIO(
      write_file, close_file, this, nullptr);
    if (uri_output == nullptr) {
      raise_warning("Unable to create output buffer");
      return false;
    }
    m_ptr = xmlNewTextWriter(uri_output);
    return true;
  }

  bool setIndentString(const String& indentString) {
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterSetIndentString(m_ptr,
                                        (xmlChar*)indentString.data());
    }
    return ret != -1;
  }

  bool setIndent(bool indent) {
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterSetIndent(m_ptr, indent);
    }
    return ret != -1;
  }

  bool startDocument(const Variant& version,
                                    const Variant& encoding,
                                    const Variant& standalone) {
    int ret = -1;
    if (m_ptr) {
      const auto pencoding = encoding.isNull() || encoding.toString().empty()
                           ? nullptr
                           : xmls(encoding);
      const auto pstandalone = standalone.isNull() ||
                               standalone.toString().empty()
                             ? nullptr
                             : xmls(standalone);
      ret = xmlTextWriterStartDocument(m_ptr,
                                       (const char *)xmls(version),
                                       (const char *)pencoding,
                                       (const char *)pstandalone);
    }
    return ret != -1;
  }

  bool startElement(const String& name) {
    if (xmlValidateName((xmlChar*)name.data(), 0)) {
      raise_warning("invalid element name: %s", name.data());
      return false;
    }
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterStartElement(m_ptr, (xmlChar*)name.data());
    }
    return ret != -1;
  }

  bool startElementNS(const Variant& prefix,
                                     const String& name,
                                     const Variant& uri) {
    if (xmlValidateName((xmlChar*)name.data(), 0)) {
      raise_warning("invalid element name: %s", name.data());
      return false;
    }
    int ret = -1;
    if (m_ptr) {
      // To be consistent with PHP5, we need to make a distinction between
      // null strings and empty strings for the prefix. We use const Variant&
      // above because null strings are coerced to empty strings automatically.
      xmlChar * prefixData = prefix.isNull()
        ? nullptr : (xmlChar *)prefix.toString().data();
      // And same for URI
      xmlChar* uriData = uri.isNull()
        ? nullptr : (xmlChar *)uri.toString().data();
      ret = xmlTextWriterStartElementNS(m_ptr, prefixData,
                                        (xmlChar*)name.data(),
                                        uriData);
    }
    return ret != -1;
  }

  bool writeElementNS(const Variant& prefix,
                                     const String& name,
                                     const Variant& uri,
                                     const Variant& content) {
    if (xmlValidateName((xmlChar*)name.data(), 0)) {
      raise_warning("invalid element name: %s", name.data());
      return false;
    }
    int ret = -1;
    if (m_ptr) {
      xmlChar* prefixData = prefix.isNull()
        ? nullptr : (xmlChar *)prefix.toString().data();
      xmlChar* uriData = uri.isNull()
        ? nullptr : (xmlChar *)uri.toString().data();
      if (content.isNull()) {
        ret = xmlTextWriterStartElementNS(m_ptr, prefixData,
                                          (xmlChar*)name.data(),
                                          uriData);
        if (ret == -1) return false;
        ret = xmlTextWriterEndElement(m_ptr);
        if (ret == -1) return false;
      } else {
        ret = xmlTextWriterWriteElementNS(m_ptr, prefixData,
                                          (xmlChar*)name.data(),
                                          uriData,
                                          (xmlChar*)content.toString().data());
      }
    }
    return ret != -1;
  }

  bool writeElement(const String& name, const Variant& content) {
    if (xmlValidateName((xmlChar*)name.data(), 0)) {
      raise_warning("invalid element name: %s", name.data());
      return false;
    }
    int ret = -1;
    if (m_ptr) {
      if (content.isNull()) {
        ret = xmlTextWriterStartElement(m_ptr, (xmlChar*)name.data());
        if (ret == -1) return false;
        ret = xmlTextWriterEndElement(m_ptr);
        if (ret == -1) return false;
      } else {
        ret = xmlTextWriterWriteElement(m_ptr, (xmlChar*)name.data(),
                                        (xmlChar*)content.toString().data());
      }
    }
    return ret != -1;
  }

  bool endElement() {
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterEndElement(m_ptr);
    }
    return ret != -1;
  }

  bool fullEndElement() {
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterFullEndElement(m_ptr);
    }
    return ret != -1;
  }

  bool startAttributeNS(const String& prefix,
                                       const String& name,
                                       const String& uri) {
    if (xmlValidateName((xmlChar*)name.data(), 0)) {
      raise_warning("invalid attribute name: %s", name.data());
      return false;
    }
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterStartAttributeNS(m_ptr, (xmlChar*)prefix.data(),
                                          (xmlChar*)name.data(),
                                          (xmlChar*)uri.data());
    }
    return ret != -1;
  }

  bool startAttribute(const String& name) {
    if (xmlValidateName((xmlChar*)name.data(), 0)) {
      raise_warning("invalid attribute name: %s", name.data());
      return false;
    }
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterStartAttribute(m_ptr, (xmlChar*)name.data());
    }
    return ret != -1;
  }

  bool writeAttributeNS(const String& prefix,
                                    const String& name,
                                    const String& uri,
                                    const String& content) {
    if (xmlValidateName((xmlChar*)name.data(), 0)) {
      raise_warning("invalid attribute name: %s", name.data());
      return false;
    }
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterWriteAttributeNS(m_ptr, (xmlChar*)prefix.data(),
                                          (xmlChar*)name.data(),
                                          (xmlChar*)uri.data(),
                                          (xmlChar*)content.data());
    }
    return ret != -1;
  }

  bool writeAttribute(const String& name, const String& value) {
    if (xmlValidateName((xmlChar*)name.data(), 0)) {
      raise_warning("invalid attribute name: %s", name.data());
      return false;
    }
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterWriteAttribute(m_ptr, (xmlChar*)name.data(),
                                        (xmlChar*)value.data());
    }
    return ret != -1;
  }

  bool endAttribute() {
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterEndAttribute(m_ptr);
    }
    return ret != -1;
  }

  bool startCData() {
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterStartCDATA(m_ptr);
    }
    return ret != -1;
  }

  bool writeCData(const String& content) {
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterWriteCDATA(m_ptr, (xmlChar*)content.data());
    }
    return ret != -1;
  }

  bool endCData() {
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterEndCDATA(m_ptr);
    }
    return ret != -1;
  }

  bool startComment() {
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterStartComment(m_ptr);
    }
    return ret != -1;
  }

  bool writeComment(const String& content) {
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterWriteComment(m_ptr, (xmlChar*)content.data());
    }
    return ret != -1;
  }

  bool endComment() {
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterEndComment(m_ptr);
    }
    return ret != -1;
  }

  bool endDocument() {
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterEndDocument(m_ptr);
    }
    return ret != -1;
  }

  bool startPI(const String& target) {
    if (xmlValidateName((xmlChar*)target.data(), 0)) {
      raise_warning("invalid PI target: %s", target.data());
      return false;
    }
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterStartPI(m_ptr, (xmlChar*)target.data());
    }
    return ret != -1;
  }

  bool writePI(const String& target, const String& content) {
    if (xmlValidateName((xmlChar*)target.data(), 0)) {
      raise_warning("invalid PI target: %s", target.data());
      return false;
    }
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterWritePI(m_ptr, (xmlChar*)target.data(),
                                 (xmlChar*)content.data());
    }
    return ret != -1;
  }

  bool endPI() {
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterEndPI(m_ptr);
    }
    return ret != -1;
  }

  bool text(const String& content) {
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterWriteString(m_ptr, (xmlChar*)content.data());
    }
    return ret != -1;
  }

  bool writeRaw(const String& content) {
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterWriteRaw(m_ptr, (xmlChar*)content.data());
    }
    return ret != -1;
  }

  bool startDTD(const String& qualifiedname,
                               const Variant& publicid,
                               const Variant& systemid) {
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterStartDTD(m_ptr, (xmlChar*)qualifiedname.data(),
                                  xmls(publicid), xmls(systemid));
    }
    return ret != -1;
  }

  bool writeDTD(const String& name,
                               const Variant& publicid,
                               const Variant& systemid,
                               const Variant& subset) {
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterWriteDTD(m_ptr, (xmlChar*)name.data(),
                                  xmls(publicid), xmls(systemid), xmls(subset));
    }
    return ret != -1;
  }

  bool startDTDElement(const String& qualifiedname) {
    if (xmlValidateName((xmlChar*)qualifiedname.data(), 0)) {
      raise_warning("invalid element name: %s", qualifiedname.data());
      return false;
    }
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterStartDTDElement(m_ptr,
                                        (xmlChar*)qualifiedname.data());
    }
    return ret != -1;
  }

  bool writeDTDElement(const String& name, const String& content) {
    if (xmlValidateName((xmlChar*)name.data(), 0)) {
      raise_warning("invalid element name: %s", name.data());
      return false;
    }
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterWriteDTDElement(m_ptr, (xmlChar*)name.data(),
                                         (xmlChar*)content.data());
    }
    return ret != -1;
  }

  bool endDTDElement() {
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterEndDTDElement(m_ptr);
    }
    return ret != -1;
  }

  bool startDTDAttlist(const String& name) {
    if (xmlValidateName((xmlChar*)name.data(), 0)) {
      raise_warning("invalid element name: %s", name.data());
      return false;
    }
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterStartDTDAttlist(m_ptr, (xmlChar*)name.data());
    }
    return ret != -1;
  }

  bool writeDTDAttlist(const String& name, const String& content) {
    if (xmlValidateName((xmlChar*)name.data(), 0)) {
      raise_warning("invalid element name: %s", name.data());
      return false;
    }
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterWriteDTDAttlist(m_ptr, (xmlChar*)name.data(),
                                         (xmlChar*)content.data());
    }
    return ret != -1;
  }

  bool endDTDAttlist() {
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterEndDTDAttlist(m_ptr);
    }
    return ret != -1;
  }

  bool startDTDEntity(const String& name, bool isparam) {
    if (xmlValidateName((xmlChar*)name.data(), 0)) {
      raise_warning("invalid attribute name: %s", name.data());
      return false;
    }
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterStartDTDEntity(m_ptr, isparam,
                                        (xmlChar*)name.data());
    }
    return ret != -1;
  }

  bool writeDTDEntity(const String& name,
                                     const String& content,
                                     bool pe,
                                     const String& publicid,
                                     const String& systemid,
                                     const String& ndataid) {
    if (xmlValidateName((xmlChar*)name.data(), 0)) {
      raise_warning("invalid element name: %s", name.data());
      return false;
    }
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterWriteDTDEntity(m_ptr, pe, (xmlChar*)name.data(),
                                        xmls(publicid), xmls(systemid),
                                        xmls(ndataid),
                                        (xmlChar*)content.data());
    }
    return ret != -1;
  }

  bool endDTDEntity() {
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterEndDTDEntity(m_ptr);
    }
    return ret != -1;
  }

  bool endDTD() {
    int ret = -1;
    if (m_ptr) {
      ret = xmlTextWriterEndDTD(m_ptr);
    }
    return ret != -1;
  }

  String flush(const Variant& empty /*= true*/) {
    if (m_ptr && m_output) {
      xmlTextWriterFlush(m_ptr);
      String ret((char*)m_output->content, CopyString);
      if (!empty.isNull() && empty.asBooleanVal()) {
        xmlBufferEmpty(m_output);
      }
      return ret;
    }
    return empty_string();
  }

  String outputMemory(const Variant& flush /*= true*/) {
    return this->flush(flush);
  }

public:
  xmlTextWriterPtr  m_ptr;
  xmlBufferPtr      m_output;
  req::ptr<File>    m_uri;

private:
///////////////////////////////////////////////////////////////////////////////
// helpers

  static int write_file(void *context, const char *buffer, int len) {
    return len > 0 ? reinterpret_cast<XMLWriterData*>(context)->m_uri->
      writeImpl(buffer, len) : 0;
  }

  static int close_file(void* /*context*/) { return 0; }

  static xmlChar *xmls(const Variant& s) {
    return s.isNull() ? nullptr : (xmlChar*)s.toString().data();
  }

};

///////////////////////////////////////////////////////////////////////////////

struct XMLWriterResource : SweepableResourceData {
private:
  DECLARE_RESOURCE_ALLOCATION(XMLWriterResource)

public:
  CLASSNAME_IS("xmlwriter")

  const String& o_getClassNameHook() const override {
    return classnameof();
  }

public:
  XMLWriterData m_writer;
};

void XMLWriterResource::sweep() {
  m_writer.sweep();
}

///////////////////////////////////////////////////////////////////////////////
// methods (object oriented style) and functions (procedural style)
#define EXTRACT_ARGS1(type1, arg1) arg1
#define EXTRACT_ARGS2(type1, arg1, type2, arg2) arg1, arg2
#define EXTRACT_ARGS3(type1, arg1, type2, arg2, type3, arg3) arg1, arg2, arg3
#define EXTRACT_ARGS4(type1, arg1, type2, arg2, type3, arg3, type4, arg4)      \
  arg1, arg2, arg3, arg4                                                       \

#define EXTRACT_ARGS5(type1, arg1, type2, arg2, type3, arg3, type4, arg4,      \
                      type5, arg5) arg1, arg2, arg3, arg4, arg5                \

#define EXTRACT_ARGS6(type1, arg1, type2, arg2, type3, arg3,                   \
                      type4, arg4, type5, arg5, type6, arg6)                   \
                      arg1, arg2, arg3, arg4, arg5, arg6                       \

#define CREATE_PARAMS1(type1, arg1) type1 arg1
#define CREATE_PARAMS2(type1, arg1, type2, arg2) type1 arg1, type2 arg2        \

#define CREATE_PARAMS3(type1, arg1, type2, arg2, type3, arg3)                  \
  type1 arg1, type2 arg2, type3 arg3                                           \

#define CREATE_PARAMS4(type1, arg1, type2, arg2, type3, arg3, type4, arg4)     \
  type1 arg1, type2 arg2, type3 arg3, type4 arg4                               \

#define CREATE_PARAMS5(type1, arg1, type2, arg2, type3, arg3, type4, arg4,     \
                       type5, arg5)                                            \
  type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5                   \

#define CREATE_PARAMS6(type1, arg1, type2, arg2, type3, arg3, type4, arg4,     \
                       type5, arg5, type6, arg6)                               \
  type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6       \

// Decide which macro to call depending on the number of arguments provided.
#define MACRO_OVERLOAD(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8,   \
                       arg9, arg10, arg11, MACRO_NAME, ...) MACRO_NAME         \

#define MACRO_OVERLOAD_(tuple) MACRO_OVERLOAD tuple

#define EXTRACT_ARGS(...) MACRO_OVERLOAD_((__VA_ARGS__,                        \
                                         EXTRACT_ARGS6, _,                     \
                                         EXTRACT_ARGS5, _,                     \
                                         EXTRACT_ARGS4, _,                     \
                                         EXTRACT_ARGS3, _,                     \
                                         EXTRACT_ARGS2, _,                     \
                                         EXTRACT_ARGS1))(__VA_ARGS__)          \

#define CREATE_PARAMS(...) MACRO_OVERLOAD_((__VA_ARGS__,                       \
                                         CREATE_PARAMS6, _,                    \
                                         CREATE_PARAMS5, _,                    \
                                         CREATE_PARAMS4, _,                    \
                                         CREATE_PARAMS3, _,                    \
                                         CREATE_PARAMS2, _,                    \
                                         CREATE_PARAMS1))(__VA_ARGS__)         \

#define XMLWRITER_METHOD(return_type, method_name, ...)                        \
  static return_type HHVM_METHOD(XMLWriter, method_name,                       \
                                 CREATE_PARAMS(__VA_ARGS__)) {                 \
    VMRegGuard _;                                                             \
    auto data = Native::data<XMLWriterData>(this_);                            \
    return data->method_name(EXTRACT_ARGS(__VA_ARGS__));                       \
  }                                                                            \

#define CHECK_RESOURCE(writer)                                                 \
  auto writer = dyn_cast_or_null<XMLWriterResource>(wr);                       \
  if (writer == nullptr) {                                                     \
    raise_warning("supplied argument is not a valid xmlwriter "                \
                  "handle resource");                                          \
    return false;                                                              \
  }                                                                            \

#define XMLWRITER_FUNCTION(return_type, function_name, method_name, ...)       \
  static return_type HHVM_FUNCTION(function_name,                              \
                         const OptResource& wr, CREATE_PARAMS(__VA_ARGS__)) {  \
    VMRegGuard _;                                                             \
    CHECK_RESOURCE(resource);                                                  \
    return resource->m_writer.method_name(EXTRACT_ARGS(__VA_ARGS__));          \
  }                                                                            \

#define XMLWRITER_METHOD_AND_FUNCTION(return_type, function_name,              \
                                      method_name, ...)                        \
  XMLWRITER_METHOD(return_type, method_name, __VA_ARGS__)                      \
  XMLWRITER_FUNCTION(return_type, function_name, method_name, __VA_ARGS__)     \

#define XMLWRITER_METHOD_NO_ARGS(return_type, method_name)                     \
  static return_type HHVM_METHOD(XMLWriter, method_name) {                     \
    VMRegGuard _;                                                             \
    auto data = Native::data<XMLWriterData>(this_);                            \
    return data->method_name();                                                \
  }                                                                            \

#define XMLWRITER_FUNCTION_NO_ARGS(return_type, function_name, method_name)    \
  static return_type HHVM_FUNCTION(function_name, const OptResource& wr) {     \
    VMRegGuard _;                                                             \
    CHECK_RESOURCE(resource);                                                  \
    return resource->m_writer.method_name();                                   \
  }                                                                            \

#define XMLWRITER_METHOD_AND_FUNCTION_NO_ARGS(return_type, function_name,      \
                                              method_name)                     \
  XMLWRITER_METHOD_NO_ARGS(return_type, method_name)                           \
  XMLWRITER_FUNCTION_NO_ARGS(return_type, function_name, method_name)          \

XMLWRITER_METHOD_NO_ARGS(bool, openMemory)

static Variant HHVM_FUNCTION(xmlwriter_open_memory) {
  auto data = req::make<XMLWriterResource>();

  bool opened = data->m_writer.openMemory();
  if (!opened) {
    return false;
  }
  return Variant(std::move(data));
}

XMLWRITER_METHOD(bool, openURI,
                 const String&, uri)

static Variant HHVM_FUNCTION(xmlwriter_open_uri,
                             const String& uri) {
  auto data = req::make<XMLWriterResource>();

  bool opened = data->m_writer.openURI(uri);
  if (!opened) {
    return false;
  }
  return Variant(std::move(data));
}

XMLWRITER_METHOD(bool, setIndentString,
                 const String&, indentString)

static Variant HHVM_FUNCTION(xmlwriter_set_indent_string,
                         const OptResource& wr, const Variant& indentString) {
  VMRegGuard _;
  CHECK_RESOURCE(resource);

  if (!indentString.isString()) {
    return false;
  }
  return resource->m_writer.setIndentString(indentString.toString());
}

XMLWRITER_METHOD_AND_FUNCTION(bool, xmlwriter_set_indent, setIndent,
                              bool, indent)

XMLWRITER_METHOD_AND_FUNCTION(bool, xmlwriter_start_document, startDocument,
                              const Variant&, version /*= "1.0"*/,
                              const Variant&, encoding /*= ""*/,
                              const Variant&, standalone /*= ""*/)

XMLWRITER_METHOD_AND_FUNCTION(bool, xmlwriter_start_element, startElement,
                              const String&, name)

XMLWRITER_METHOD_AND_FUNCTION(bool, xmlwriter_start_element_ns, startElementNS,
                              const Variant&, prefix,
                              const String&, name,
                              const Variant&, uri)

XMLWRITER_METHOD_AND_FUNCTION(bool, xmlwriter_write_element_ns, writeElementNS,
                              const Variant&, prefix,
                              const String&, name,
                              const Variant&, uri,
                              const Variant&, content)

XMLWRITER_METHOD_AND_FUNCTION(bool, xmlwriter_write_element, writeElement,
                              const String&, name,
                              const Variant&, content)

XMLWRITER_METHOD_AND_FUNCTION_NO_ARGS(bool, xmlwriter_end_element, endElement)

XMLWRITER_METHOD_AND_FUNCTION_NO_ARGS(bool, xmlwriter_full_end_element,
                                      fullEndElement)

XMLWRITER_METHOD_AND_FUNCTION(bool, xmlwriter_start_attribute_ns,
                              startAttributeNS,
                              const String&, prefix,
                              const String&, name,
                              const String&, uri)

XMLWRITER_METHOD_AND_FUNCTION(bool, xmlwriter_start_attribute, startAttribute,
                              const String&, name)

XMLWRITER_METHOD_AND_FUNCTION(bool, xmlwriter_write_attribute_ns,
                              writeAttributeNS,
                              const String&, prefix,
                              const String&, name,
                              const String&, uri,
                              const String&, content)

XMLWRITER_METHOD_AND_FUNCTION(bool, xmlwriter_write_attribute, writeAttribute,
                              const String&, name,
                              const String&, value)

XMLWRITER_METHOD_AND_FUNCTION_NO_ARGS(bool, xmlwriter_end_attribute,
                                      endAttribute)

XMLWRITER_METHOD_AND_FUNCTION_NO_ARGS(bool, xmlwriter_start_cdata, startCData)

XMLWRITER_METHOD_AND_FUNCTION(bool, xmlwriter_write_cdata, writeCData,
                              const String&, content)

XMLWRITER_METHOD_AND_FUNCTION_NO_ARGS(bool, xmlwriter_end_cdata, endCData)

XMLWRITER_METHOD_AND_FUNCTION_NO_ARGS(bool, xmlwriter_start_comment,
                                      startComment)

XMLWRITER_METHOD_AND_FUNCTION(bool, xmlwriter_write_comment, writeComment,
                              const String&, content)

XMLWRITER_METHOD_AND_FUNCTION_NO_ARGS(bool, xmlwriter_end_comment, endComment)

XMLWRITER_METHOD_AND_FUNCTION_NO_ARGS(bool, xmlwriter_end_document, endDocument)

XMLWRITER_METHOD_AND_FUNCTION(bool, xmlwriter_start_pi, startPI,
                              const String&, target)

XMLWRITER_METHOD_AND_FUNCTION(bool, xmlwriter_write_pi, writePI,
                              const String&, target,
                              const String&, content)

XMLWRITER_METHOD_AND_FUNCTION_NO_ARGS(bool, xmlwriter_end_pi, endPI)

XMLWRITER_METHOD_AND_FUNCTION(bool, xmlwriter_text, text,
                              const String&, content)

XMLWRITER_METHOD_AND_FUNCTION(bool, xmlwriter_write_raw, writeRaw,
                              const String&, content)

XMLWRITER_METHOD_AND_FUNCTION(bool, xmlwriter_start_dtd, startDTD,
                              const String&, qualifiedname,
                              const Variant&, publicid /*= ""*/,
                              const Variant&, systemid /*= ""*/)

XMLWRITER_METHOD_AND_FUNCTION(bool, xmlwriter_write_dtd, writeDTD,
                              const String&, name,
                              const Variant&, publicid /*= ""*/,
                              const Variant&, systemid /*= ""*/,
                              const Variant&, subset /*= ""*/)

XMLWRITER_METHOD_AND_FUNCTION(bool, xmlwriter_start_dtd_element,
                              startDTDElement,
                              const String&, qualifiedname)

XMLWRITER_METHOD_AND_FUNCTION(bool, xmlwriter_write_dtd_element,
                              writeDTDElement,
                              const String&, name,
                              const String&, content)

XMLWRITER_METHOD_AND_FUNCTION_NO_ARGS(bool, xmlwriter_end_dtd_element,
                              endDTDElement)

XMLWRITER_METHOD_AND_FUNCTION(bool, xmlwriter_start_dtd_attlist,
                              startDTDAttlist,
                              const String&, name)

XMLWRITER_METHOD_AND_FUNCTION(bool, xmlwriter_write_dtd_attlist,
                              writeDTDAttlist,
                              const String&, name,
                              const String&, content)

XMLWRITER_METHOD_AND_FUNCTION_NO_ARGS(bool, xmlwriter_end_dtd_attlist,
                              endDTDAttlist)

XMLWRITER_METHOD_AND_FUNCTION(bool, xmlwriter_start_dtd_entity, startDTDEntity,
                              const String&, name,
                              bool, isparam)

XMLWRITER_METHOD_AND_FUNCTION(bool, xmlwriter_write_dtd_entity, writeDTDEntity,
                              const String&, name,
                              const String&, content,
                              bool, pe /*= false*/,
                              const String&, publicid /*= ""*/,
                              const String&, systemid /*= ""*/,
                              const String&, ndataid /*= ""*/)

XMLWRITER_METHOD_AND_FUNCTION_NO_ARGS(bool, xmlwriter_end_dtd_entity,
                                      endDTDEntity)

XMLWRITER_METHOD_AND_FUNCTION_NO_ARGS(bool, xmlwriter_end_dtd, endDTD)

XMLWRITER_METHOD_AND_FUNCTION(Variant, xmlwriter_flush, flush,
                              const Variant&, empty /*= true*/)

XMLWRITER_METHOD_AND_FUNCTION(String, xmlwriter_output_memory, outputMemory,
                              const Variant&, flush /*= true*/)

///////////////////////////////////////////////////////////////////////////////
// extension
struct XMLWriterExtension final : Extension {
    XMLWriterExtension() : Extension("xmlwriter", "0.1", NO_ONCALL_YET) {};

    void moduleInit() override {
      HHVM_ME(XMLWriter, openMemory);
      HHVM_ME(XMLWriter, openURI);
      HHVM_ME(XMLWriter, setIndentString);
      HHVM_ME(XMLWriter, setIndent);
      HHVM_ME(XMLWriter, startDocument);
      HHVM_ME(XMLWriter, startElement);
      HHVM_ME(XMLWriter, startElementNS);
      HHVM_ME(XMLWriter, writeElementNS);
      HHVM_ME(XMLWriter, writeElement);
      HHVM_ME(XMLWriter, endElement);
      HHVM_ME(XMLWriter, fullEndElement);
      HHVM_ME(XMLWriter, startAttributeNS);
      HHVM_ME(XMLWriter, startAttribute);
      HHVM_ME(XMLWriter, writeAttributeNS);
      HHVM_ME(XMLWriter, writeAttribute);
      HHVM_ME(XMLWriter, endAttribute);
      HHVM_ME(XMLWriter, startCData);
      HHVM_ME(XMLWriter, writeCData);
      HHVM_ME(XMLWriter, endCData);
      HHVM_ME(XMLWriter, startComment);
      HHVM_ME(XMLWriter, writeComment);
      HHVM_ME(XMLWriter, endComment);
      HHVM_ME(XMLWriter, endDocument);
      HHVM_ME(XMLWriter, startPI);
      HHVM_ME(XMLWriter, writePI);
      HHVM_ME(XMLWriter, endPI);
      HHVM_ME(XMLWriter, text);
      HHVM_ME(XMLWriter, writeRaw);
      HHVM_ME(XMLWriter, startDTD);
      HHVM_ME(XMLWriter, writeDTD);
      HHVM_ME(XMLWriter, startDTDElement);
      HHVM_ME(XMLWriter, writeDTDElement);
      HHVM_ME(XMLWriter, endDTDElement);
      HHVM_ME(XMLWriter, startDTDAttlist);
      HHVM_ME(XMLWriter, writeDTDAttlist);
      HHVM_ME(XMLWriter, endDTDAttlist);
      HHVM_ME(XMLWriter, startDTDEntity);
      HHVM_ME(XMLWriter, writeDTDEntity);
      HHVM_ME(XMLWriter, endDTDEntity);
      HHVM_ME(XMLWriter, endDTD);
      HHVM_ME(XMLWriter, flush);
      HHVM_ME(XMLWriter, outputMemory);

      HHVM_FE(xmlwriter_open_memory);
      HHVM_FE(xmlwriter_open_uri);
      HHVM_FE(xmlwriter_set_indent_string);
      HHVM_FE(xmlwriter_set_indent);
      HHVM_FE(xmlwriter_start_document);
      HHVM_FE(xmlwriter_start_element);
      HHVM_FE(xmlwriter_start_element_ns);
      HHVM_FE(xmlwriter_write_element_ns);
      HHVM_FE(xmlwriter_write_element);
      HHVM_FE(xmlwriter_end_element);
      HHVM_FE(xmlwriter_full_end_element);
      HHVM_FE(xmlwriter_start_attribute_ns);
      HHVM_FE(xmlwriter_start_attribute);
      HHVM_FE(xmlwriter_write_attribute_ns);
      HHVM_FE(xmlwriter_write_attribute);
      HHVM_FE(xmlwriter_end_attribute);
      HHVM_FE(xmlwriter_start_cdata);
      HHVM_FE(xmlwriter_write_cdata);
      HHVM_FE(xmlwriter_end_cdata);
      HHVM_FE(xmlwriter_start_comment);
      HHVM_FE(xmlwriter_write_comment);
      HHVM_FE(xmlwriter_end_comment);
      HHVM_FE(xmlwriter_end_document);
      HHVM_FE(xmlwriter_start_pi);
      HHVM_FE(xmlwriter_write_pi);
      HHVM_FE(xmlwriter_end_pi);
      HHVM_FE(xmlwriter_text);
      HHVM_FE(xmlwriter_write_raw);
      HHVM_FE(xmlwriter_start_dtd);
      HHVM_FE(xmlwriter_write_dtd);
      HHVM_FE(xmlwriter_start_dtd_element);
      HHVM_FE(xmlwriter_write_dtd_element);
      HHVM_FE(xmlwriter_end_dtd_element);
      HHVM_FE(xmlwriter_start_dtd_attlist);
      HHVM_FE(xmlwriter_write_dtd_attlist);
      HHVM_FE(xmlwriter_end_dtd_attlist);
      HHVM_FE(xmlwriter_start_dtd_entity);
      HHVM_FE(xmlwriter_write_dtd_entity);
      HHVM_FE(xmlwriter_end_dtd_entity);
      HHVM_FE(xmlwriter_end_dtd);
      HHVM_FE(xmlwriter_flush);
      HHVM_FE(xmlwriter_output_memory);

      Native::registerNativeDataInfo<XMLWriterData>(s_XMLWriter.get());
    }
} s_xmlwriter_extension;


///////////////////////////////////////////////////////////////////////////////
}
