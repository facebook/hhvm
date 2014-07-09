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

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/vm/native-data.h"

#include <libxml/tree.h>
#include <libxml/xmlwriter.h>
#include <libxml/uri.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
const StaticString s_XMLWriterData("XMLWriterData");

class XMLWriterData : public Sweepable {
  public:
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

  public:
    xmlTextWriterPtr  m_ptr;
    xmlBufferPtr      m_output;
    Resource          m_uri;
};

///////////////////////////////////////////////////////////////////////////////
// helpers

static int write_file(void *context, const char *buffer, int len) {
  return len > 0 ? ((XMLWriterData*)context)->m_uri.getTyped<File>()->
    writeImpl(buffer, len) : 0;
}

static int close_file(void *context) {
  return 0;
}

static xmlChar *xmls(const Variant& s) {
  return s.isNull() ? nullptr : (xmlChar*)s.toString().data();
}

///////////////////////////////////////////////////////////////////////////////
// methods

static bool HHVM_METHOD(XMLWriter, openMemory) {
  auto data = Native::data<XMLWriterData>(this_);

  data->m_output = xmlBufferCreate();
  if (data->m_output == nullptr) {
    raise_warning("Unable to create output buffer");
    return false;
  }
  return (data->m_ptr = xmlNewTextWriterMemory(data->m_output, 0));
}

static bool HHVM_METHOD(XMLWriter, openURI,
                        const String& uri) {
  auto data = Native::data<XMLWriterData>(this_);

  data->m_uri = File::Open(uri, "wb");
  if (data->m_uri.isNull()) {
    return false;
  }

  xmlOutputBufferPtr uri_output = xmlOutputBufferCreateIO(
    write_file, close_file, data, nullptr);
  if (uri_output == nullptr) {
    raise_warning("Unable to create output buffer");
    return false;
  }
  data->m_ptr = xmlNewTextWriter(uri_output);
  return true;
}

static bool HHVM_METHOD(XMLWriter, setIndentString,
                        const String& indentstring) {
  auto data = Native::data<XMLWriterData>(this_);

  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterSetIndentString(data->m_ptr,
                                      (xmlChar*)indentstring.data());
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, setIndent,
                        bool indent) {
  auto data = Native::data<XMLWriterData>(this_);

  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterSetIndent(data->m_ptr, indent);
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, startDocument,
                        const Variant& version /*= "1.0"*/,
                        const Variant& encoding /*= ""*/,
                        const Variant& standalone /*= ""*/) {
  auto data = Native::data<XMLWriterData>(this_);

  int ret = -1;
  if (data->m_ptr) {
    const auto pencoding = encoding.isNull() || encoding.toString().empty()
                         ? nullptr
                         : xmls(encoding);
    const auto pstandalone = standalone.isNull() ||
                             standalone.toString().empty()
                           ? nullptr
                           : xmls(standalone);
    ret = xmlTextWriterStartDocument(data->m_ptr,
                                     (const char *)xmls(version),
                                     (const char *)pencoding,
                                     (const char *)pstandalone);
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, startElement,
                        const String& name) {
  auto data = Native::data<XMLWriterData>(this_);

  if (xmlValidateName((xmlChar*)name.data(), 0)) {
    raise_warning("invalid element name: %s", name.data());
    return false;
  }
  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterStartElement(data->m_ptr, (xmlChar*)name.data());
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, startElementNS,
                        const Variant& prefix,
                        const String& name,
                        const Variant& uri) {
  auto data = Native::data<XMLWriterData>(this_);

  if (xmlValidateName((xmlChar*)name.data(), 0)) {
    raise_warning("invalid element name: %s", name.data());
    return false;
  }
  int ret = -1;
  if (data->m_ptr) {
    // To be consistent with PHP5, we need to make a distinction between
    // null strings and empty strings for the prefix. We use const Variant&
    // above because null strings are coerced to empty strings automatically.
    xmlChar * prefixData = prefix.isNull()
      ? nullptr : (xmlChar *)prefix.toString().data();
    // And same for URI
    xmlChar* uriData = uri.isNull()
      ? nullptr : (xmlChar *)uri.toString().data();
    ret = xmlTextWriterStartElementNS(data->m_ptr, prefixData,
                                      (xmlChar*)name.data(),
                                      uriData);
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, writeElementNS,
                        const Variant& prefix,
                        const String& name,
                        const Variant& uri,
                        const Variant& content) {
  auto data = Native::data<XMLWriterData>(this_);

  if (xmlValidateName((xmlChar*)name.data(), 0)) {
    raise_warning("invalid element name: %s", name.data());
    return false;
  }
  int ret = -1;
  if (data->m_ptr) {
    xmlChar* prefixData = prefix.isNull()
      ? nullptr : (xmlChar *)prefix.toString().data();
    xmlChar* uriData = uri.isNull()
      ? nullptr : (xmlChar *)uri.toString().data();
    if (content.isNull()) {
      ret = xmlTextWriterStartElementNS(data->m_ptr, prefixData,
                                        (xmlChar*)name.data(),
                                        uriData);
      if (ret == -1) return false;
      ret = xmlTextWriterEndElement(data->m_ptr);
      if (ret == -1) return false;
    } else {
      ret = xmlTextWriterWriteElementNS(data->m_ptr, prefixData,
                                        (xmlChar*)name.data(),
                                        uriData,
                                        (xmlChar*)content.toString().data());
    }
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, writeElement,
                        const String& name,
                        const Variant& content) {
  auto data = Native::data<XMLWriterData>(this_);

  if (xmlValidateName((xmlChar*)name.data(), 0)) {
    raise_warning("invalid element name: %s", name.data());
    return false;
  }
  int ret = -1;
  if (data->m_ptr) {
    if (content.isNull()) {
      ret = xmlTextWriterStartElement(data->m_ptr, (xmlChar*)name.data());
      if (ret == -1) return false;
      ret = xmlTextWriterEndElement(data->m_ptr);
      if (ret == -1) return false;
    } else {
      ret = xmlTextWriterWriteElement(data->m_ptr, (xmlChar*)name.data(),
                                      (xmlChar*)content.toString().data());
    }
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, endElement) {
  auto data = Native::data<XMLWriterData>(this_);

  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterEndElement(data->m_ptr);
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, fullEndElement) {
  auto data = Native::data<XMLWriterData>(this_);

  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterFullEndElement(data->m_ptr);
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, startAttributeNS,
                        const String& prefix,
                        const String& name,
                        const String& uri) {
  auto data = Native::data<XMLWriterData>(this_);

  if (xmlValidateName((xmlChar*)name.data(), 0)) {
    raise_warning("invalid attribute name: %s", name.data());
    return false;
  }
  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterStartAttributeNS(data->m_ptr, (xmlChar*)prefix.data(),
                                        (xmlChar*)name.data(),
                                        (xmlChar*)uri.data());
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, startAttribute,
                        const String& name) {
  auto data = Native::data<XMLWriterData>(this_);

  if (xmlValidateName((xmlChar*)name.data(), 0)) {
    raise_warning("invalid attribute name: %s", name.data());
    return false;
  }
  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterStartAttribute(data->m_ptr, (xmlChar*)name.data());
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, writeAttributeNS,
                        const String& prefix,
                        const String& name,
                        const String& uri,
                        const String& content) {
  auto data = Native::data<XMLWriterData>(this_);

  if (xmlValidateName((xmlChar*)name.data(), 0)) {
    raise_warning("invalid attribute name: %s", name.data());
    return false;
  }
  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterWriteAttributeNS(data->m_ptr, (xmlChar*)prefix.data(),
                                        (xmlChar*)name.data(),
                                        (xmlChar*)uri.data(),
                                        (xmlChar*)content.data());
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, writeAttribute,
                        const String& name,
                        const String& value) {
  auto data = Native::data<XMLWriterData>(this_);

  if (xmlValidateName((xmlChar*)name.data(), 0)) {
    raise_warning("invalid attribute name: %s", name.data());
    return false;
  }
  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterWriteAttribute(data->m_ptr, (xmlChar*)name.data(),
                                      (xmlChar*)value.data());
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, endAttribute) {
  auto data = Native::data<XMLWriterData>(this_);

  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterEndAttribute(data->m_ptr);
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, startCData) {
  auto data = Native::data<XMLWriterData>(this_);

  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterStartCDATA(data->m_ptr);
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, writeCData,
                        const String& content) {
  auto data = Native::data<XMLWriterData>(this_);

  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterWriteCDATA(data->m_ptr, (xmlChar*)content.data());
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, endCData) {
  auto data = Native::data<XMLWriterData>(this_);

  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterEndCDATA(data->m_ptr);
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, startComment) {
  auto data = Native::data<XMLWriterData>(this_);

  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterStartComment(data->m_ptr);
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, writeComment,
                        const String& content) {
  auto data = Native::data<XMLWriterData>(this_);

  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterWriteComment(data->m_ptr, (xmlChar*)content.data());
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, endComment) {
  auto data = Native::data<XMLWriterData>(this_);

  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterEndComment(data->m_ptr);
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, endDocument) {
  auto data = Native::data<XMLWriterData>(this_);

  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterEndDocument(data->m_ptr);
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, startPI,
                        const String& target) {
  auto data = Native::data<XMLWriterData>(this_);

  if (xmlValidateName((xmlChar*)target.data(), 0)) {
    raise_warning("invalid PI target: %s", target.data());
    return false;
  }
  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterStartPI(data->m_ptr, (xmlChar*)target.data());
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, writePI,
                        const String& target,
                        const String& content) {
  auto data = Native::data<XMLWriterData>(this_);

  if (xmlValidateName((xmlChar*)target.data(), 0)) {
    raise_warning("invalid PI target: %s", target.data());
    return false;
  }
  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterWritePI(data->m_ptr, (xmlChar*)target.data(),
                               (xmlChar*)content.data());
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, endPI) {
  auto data = Native::data<XMLWriterData>(this_);

  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterEndPI(data->m_ptr);
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, text,
                        const String& content) {
  auto data = Native::data<XMLWriterData>(this_);

  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterWriteString(data->m_ptr, (xmlChar*)content.data());
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, writeRaw,
                        const String& content) {
  auto data = Native::data<XMLWriterData>(this_);

  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterWriteRaw(data->m_ptr, (xmlChar*)content.data());
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, startDTD,
                        const String& qualifiedname,
                        const Variant& publicid /*= ""*/,
                        const Variant& systemid /*= ""*/) {
  auto data = Native::data<XMLWriterData>(this_);

  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterStartDTD(data->m_ptr, (xmlChar*)qualifiedname.data(),
                                xmls(publicid), xmls(systemid));
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, writeDTD,
                        const String& name,
                        const Variant& publicid /*= ""*/,
                        const Variant& systemid /*= ""*/,
                        const Variant& subset /*= ""*/) {
  auto data = Native::data<XMLWriterData>(this_);

  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterWriteDTD(data->m_ptr, (xmlChar*)name.data(),
                                xmls(publicid), xmls(systemid), xmls(subset));
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, startDTDElement,
                        const String& qualifiedname) {
  auto data = Native::data<XMLWriterData>(this_);

  if (xmlValidateName((xmlChar*)qualifiedname.data(), 0)) {
    raise_warning("invalid element name: %s", qualifiedname.data());
    return false;
  }
  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterStartDTDElement(data->m_ptr,
                                      (xmlChar*)qualifiedname.data());
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, writeDTDElement,
                        const String& name,
                        const String& content) {
  auto data = Native::data<XMLWriterData>(this_);

  if (xmlValidateName((xmlChar*)name.data(), 0)) {
    raise_warning("invalid element name: %s", name.data());
    return false;
  }
  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterWriteDTDElement(data->m_ptr, (xmlChar*)name.data(),
                                       (xmlChar*)content.data());
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, endDTDElement) {
  auto data = Native::data<XMLWriterData>(this_);

  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterEndDTDElement(data->m_ptr);
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, startDTDAttlist,
                        const String& name) {
  auto data = Native::data<XMLWriterData>(this_);

  if (xmlValidateName((xmlChar*)name.data(), 0)) {
    raise_warning("invalid element name: %s", name.data());
    return false;
  }
  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterStartDTDAttlist(data->m_ptr, (xmlChar*)name.data());
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, writeDTDAttlist,
                        const String& name,
                        const String& content) {
  auto data = Native::data<XMLWriterData>(this_);

  if (xmlValidateName((xmlChar*)name.data(), 0)) {
    raise_warning("invalid element name: %s", name.data());
    return false;
  }
  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterWriteDTDAttlist(data->m_ptr, (xmlChar*)name.data(),
                                       (xmlChar*)content.data());
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, endDTDAttlist) {
  auto data = Native::data<XMLWriterData>(this_);

  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterEndDTDAttlist(data->m_ptr);
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, startDTDEntity,
                        const String& name,
                        bool isparam) {
  auto data = Native::data<XMLWriterData>(this_);

  if (xmlValidateName((xmlChar*)name.data(), 0)) {
    raise_warning("invalid attribute name: %s", name.data());
    return false;
  }
  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterStartDTDEntity(data->m_ptr, isparam,
                                      (xmlChar*)name.data());
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, writeDTDEntity,
                        const String& name,
                        const String& content,
                        bool pe /*= false*/,
                        const String& publicid /*= ""*/,
                        const String& systemid /*= ""*/,
                        const String& ndataid /*= ""*/) {
  auto data = Native::data<XMLWriterData>(this_);

  if (xmlValidateName((xmlChar*)name.data(), 0)) {
    raise_warning("invalid element name: %s", name.data());
    return false;
  }
  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterWriteDTDEntity(data->m_ptr, pe, (xmlChar*)name.data(),
                                      xmls(publicid), xmls(systemid),
                                      xmls(ndataid), (xmlChar*)content.data());
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, endDTDEntity) {
  auto data = Native::data<XMLWriterData>(this_);

  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterEndDTDEntity(data->m_ptr);
  }
  return ret != -1;
}

static bool HHVM_METHOD(XMLWriter, endDTD) {
  auto data = Native::data<XMLWriterData>(this_);

  int ret = -1;
  if (data->m_ptr) {
    ret = xmlTextWriterEndDTD(data->m_ptr);
  }
  return ret != -1;
}

static Variant HHVM_METHOD(XMLWriter, flush,
                        const Variant& empty /*= true*/) {
  auto data = Native::data<XMLWriterData>(this_);

  if (data->m_ptr && data->m_output) {
    xmlTextWriterFlush(data->m_ptr);
    String ret((char*)data->m_output->content, CopyString);
    if (!empty.isNull() && empty.asBooleanVal()) {
      xmlBufferEmpty(data->m_output);
    }
    return ret;
  }
  return empty_string_variant();
}

static String HHVM_METHOD(XMLWriter, outputMemory,
                        const Variant& flush /*= true*/) {
  return HHVM_MN(XMLWriter, flush)(this_, flush);
}

///////////////////////////////////////////////////////////////////////////////
// extension
class XMLWriterExtension : public Extension {
  public:
    XMLWriterExtension() : Extension("xmlwriter", "0.1") {};

    virtual void moduleInit() {
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

      Native::registerNativeDataInfo<XMLWriterData>(s_XMLWriterData.get());

      loadSystemlib();
    }
} s_xmlwriter_extension;


///////////////////////////////////////////////////////////////////////////////
}
