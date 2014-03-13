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

#include "hphp/runtime/ext/ext_xmlwriter.h"

#include "hphp/system/systemlib.h"

namespace HPHP {

IMPLEMENT_DEFAULT_EXTENSION_VERSION(xmlwriter, 0.1);

///////////////////////////////////////////////////////////////////////////////
// functions are just wrappers of object methods

Variant f_xmlwriter_open_memory() {
  c_XMLWriter *x = NEWOBJ(c_XMLWriter)();
  Object ret(x);
  if (x->t_openmemory()) {
    return ret;
  }
  return false;
}

Variant f_xmlwriter_open_uri(const String& uri) {
  c_XMLWriter *x = NEWOBJ(c_XMLWriter)();
  Object ret(x);
  if (x->t_openuri(uri)) {
    return ret;
  }
  return false;
}

bool f_xmlwriter_set_indent_string(const Object& xmlwriter, const String& indentstring) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_setindentstring(indentstring);
}

bool f_xmlwriter_set_indent(const Object& xmlwriter, bool indent) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_setindent(indent);
}

bool f_xmlwriter_start_document(const Object& xmlwriter,
                                const String& version /* = "1.0" */,
                                const String& encoding /* = null_string */,
                                const String& standalone /* = null_string */) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_startdocument(version, encoding, standalone);
}

bool f_xmlwriter_start_element(const Object& xmlwriter, const String& name) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_startelement(name);
}

bool f_xmlwriter_start_element_ns(const Object& xmlwriter,
                                  const Variant& prefix,
                                  const String& name,
                                  const String& uri) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_startelementns(prefix, name, uri);
}

bool f_xmlwriter_write_element_ns(const Object& xmlwriter, const String& prefix,
                                  const String& name, const String& uri,
                                  const String& content /* = null_string */) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_writeelementns(prefix, name, uri, content);
}

bool f_xmlwriter_write_element(const Object& xmlwriter, const String& name,
                               const String& content /* = null_string */) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_writeelement(name, content);
}

bool f_xmlwriter_end_element(const Object& xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_endelement();
}

bool f_xmlwriter_full_end_element(const Object& xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_fullendelement();
}

bool f_xmlwriter_start_attribute_ns(const Object& xmlwriter, const String& prefix,
                                    const String& name, const String& uri) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_startattributens(prefix, name, uri);
}

bool f_xmlwriter_start_attribute(const Object& xmlwriter, const String& name) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_startattribute(name);
}

bool f_xmlwriter_write_attribute_ns(const Object& xmlwriter, const String& prefix,
                                    const String& name, const String& uri,
                                    const String& content) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_writeattributens(prefix, name, uri, content);
}

bool f_xmlwriter_write_attribute(const Object& xmlwriter, const String& name,
                                 const String& value) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_writeattribute(name, value);
}

bool f_xmlwriter_end_attribute(const Object& xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_endattribute();
}

bool f_xmlwriter_start_cdata(const Object& xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_startcdata();
}

bool f_xmlwriter_write_cdata(const Object& xmlwriter, const String& content) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_writecdata(content);
}

bool f_xmlwriter_end_cdata(const Object& xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_endcdata();
}

bool f_xmlwriter_start_comment(const Object& xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_startcomment();
}

bool f_xmlwriter_write_comment(const Object& xmlwriter, const String& content) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_writecomment(content);
}

bool f_xmlwriter_end_comment(const Object& xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_endcomment();
}

bool f_xmlwriter_end_document(const Object& xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_enddocument();
}

bool f_xmlwriter_start_pi(const Object& xmlwriter, const String& target) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_startpi(target);
}

bool f_xmlwriter_write_pi(const Object& xmlwriter, const String& target, const String& content) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_writepi(target, content);
}

bool f_xmlwriter_end_pi(const Object& xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_endpi();
}

bool f_xmlwriter_text(const Object& xmlwriter, const String& content) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_text(content);
}

bool f_xmlwriter_write_raw(const Object& xmlwriter, const String& content) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_writeraw(content);
}

bool f_xmlwriter_start_dtd(const Object& xmlwriter, const String& qualifiedname,
                           const String& publicid /* = null_string */,
                           const String& systemid /* = null_string */) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_startdtd(qualifiedname, publicid, systemid);
}

bool f_xmlwriter_write_dtd(const Object& xmlwriter, const String& name,
                           const String& publicid /* = null_string */,
                           const String& systemid /* = null_string */,
                           const String& subset /* = null_string */) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_writedtd(name, publicid, systemid, subset);
}

bool f_xmlwriter_start_dtd_element(const Object& xmlwriter, const String& qualifiedname) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_startdtdelement(qualifiedname);
}

bool f_xmlwriter_write_dtd_element(const Object& xmlwriter, const String& name,
                                   const String& content) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_writedtdelement(name, content);
}

bool f_xmlwriter_end_dtd_element(const Object& xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_enddtdelement();
}

bool f_xmlwriter_start_dtd_attlist(const Object& xmlwriter, const String& name) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_startdtdattlist(name);
}

bool f_xmlwriter_write_dtd_attlist(const Object& xmlwriter, const String& name,
                                   const String& content) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_writedtdattlist(name, content);
}

bool f_xmlwriter_end_dtd_attlist(const Object& xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_enddtdattlist();
}

bool f_xmlwriter_start_dtd_entity(const Object& xmlwriter, const String& name,
                                  bool isparam) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_startdtdentity(name, isparam);
}

bool f_xmlwriter_write_dtd_entity(const Object& xmlwriter, const String& name,
                                  const String& content, bool pe /* = false */,
                                  const String& publicid /* = null_string */,
                                  const String& systemid /* = null_string */,
                                  const String& ndataid /* = null_string */) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_writedtdentity(name, content);
}

bool f_xmlwriter_end_dtd_entity(const Object& xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_enddtdentity();
}

bool f_xmlwriter_end_dtd(const Object& xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_enddtd();
}

Variant f_xmlwriter_flush(const Object& xmlwriter, bool empty /* = true */) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_flush(empty);
}

String f_xmlwriter_output_memory(const Object& xmlwriter, bool flush /* = true */) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_outputmemory(flush);
}

///////////////////////////////////////////////////////////////////////////////
// helpers

static int write_file(void *context, const char *buffer, int len) {
  return len > 0 ? ((c_XMLWriter*)context)->m_uri->writeImpl(buffer, len) : 0;
}

static int close_file(void *context) {
  return 0;
}

static xmlChar *xmls(const String& s) {
  return s.isNull() ? NULL : (xmlChar*)s.data();
}

///////////////////////////////////////////////////////////////////////////////

c_XMLWriter::c_XMLWriter(Class* cb) :
    ExtObjectData(cb), m_ptr(nullptr), m_output(nullptr) {
}

void c_XMLWriter::sweep() {
  if (m_ptr) {
    assert(m_ptr != xmlTextWriterPtr(-1));
    xmlFreeTextWriter(m_ptr);
    if (debug) {
      m_ptr = xmlTextWriterPtr(-1);
    }
  }
  if (m_output) {
    assert(m_output != xmlBufferPtr(-1));
    xmlBufferFree(m_output);
    if (debug) {
      m_output = xmlBufferPtr(-1);
    }
  }
}

c_XMLWriter::~c_XMLWriter() {
  sweep();
}

void c_XMLWriter::t___construct() {
}

bool c_XMLWriter::t_openmemory() {
  m_output = xmlBufferCreate();
  if (m_output == NULL) {
    raise_warning("Unable to create output buffer");
    return false;
  }
  return (m_ptr = xmlNewTextWriterMemory(m_output, 0));
}

bool c_XMLWriter::t_openuri(const String& uri) {
  Variant file = File::Open(uri, "wb");
  if (same(file, false)) {
    return false;
  }
  m_uri = file.toResource().getTyped<File>();

  xmlOutputBufferPtr uri_output = xmlOutputBufferCreateIO(
    write_file, close_file, this, nullptr);
  if (uri_output == nullptr) {
    raise_warning("Unable to create output buffer");
    return false;
  }
  m_ptr = xmlNewTextWriter(uri_output);
  return true;
}

bool c_XMLWriter::t_setindentstring(const String& indentstring) {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterSetIndentString(m_ptr, (xmlChar*)indentstring.data());
  }
  return ret != -1;
}

bool c_XMLWriter::t_setindent(bool indent) {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterSetIndent(m_ptr, indent);
  }
  return ret != -1;
}

bool c_XMLWriter::t_startdocument(const String& version /* = "1.0" */,
                                  const String& encoding /* = null_string */,
                                  const String& standalone /* = null_string */) {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterStartDocument(m_ptr, (const char *)xmls(version),
                                     (const char *)xmls(encoding),
                                     (const char *)xmls(standalone));
  }
  return ret != -1;
}

bool c_XMLWriter::t_startelement(const String& name) {
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

bool c_XMLWriter::t_startelementns(const Variant& prefix, const String& name,
                                   const String& uri) {
  if (xmlValidateName((xmlChar*)name.data(), 0)) {
    raise_warning("invalid element name: %s", name.data());
    return false;
  }
  int ret = -1;
  if (m_ptr) {
    // To be consistent with Zend PHP, we need to make a distinction between
    // null strings and empty strings for the prefix. We use const Variant& above
    // because null strings are coerced to empty strings automatically.
    xmlChar * prefixData = prefix.isNull()
      ? nullptr : (xmlChar *)prefix.toString().data();
    ret = xmlTextWriterStartElementNS(m_ptr, prefixData,
                                      (xmlChar*)name.data(),
                                      (xmlChar*)uri.data());
  }
  return ret != -1;
}

bool c_XMLWriter::t_writeelementns(const String& prefix, const String& name, const String& uri,
                                   const String& content /* = null_string */) {
  if (xmlValidateName((xmlChar*)name.data(), 0)) {
    raise_warning("invalid element name: %s", name.data());
    return false;
  }
  int ret = -1;
  if (m_ptr) {
    if (content.isNull()) {
      ret = xmlTextWriterStartElementNS(m_ptr, (xmlChar*)prefix.data(),
                                        (xmlChar*)name.data(),
                                        (xmlChar*)uri.data());
      if (ret == -1) return false;
      ret = xmlTextWriterEndElement(m_ptr);
      if (ret == -1) return false;
    } else {
      ret = xmlTextWriterWriteElementNS(m_ptr, (xmlChar*)prefix.data(),
                                        (xmlChar*)name.data(),
                                        (xmlChar*)uri.data(),
                                        (xmlChar*)content.data());
    }
  }
  return ret != -1;
}

bool c_XMLWriter::t_writeelement(const String& name,
                                 const String& content /* = null_string */) {
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
                                      (xmlChar*)content.data());
    }
  }
  return ret != -1;
}

bool c_XMLWriter::t_endelement() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndElement(m_ptr);
  }
  return ret != -1;
}

bool c_XMLWriter::t_fullendelement() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterFullEndElement(m_ptr);
  }
  return ret != -1;
}

bool c_XMLWriter::t_startattributens(const String& prefix, const String& name,
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

bool c_XMLWriter::t_startattribute(const String& name) {
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

bool c_XMLWriter::t_writeattributens(const String& prefix, const String& name, const String& uri,
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

bool c_XMLWriter::t_writeattribute(const String& name, const String& value) {
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

bool c_XMLWriter::t_endattribute() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndAttribute(m_ptr);
  }
  return ret != -1;
}

bool c_XMLWriter::t_startcdata() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterStartCDATA(m_ptr);
  }
  return ret != -1;
}

bool c_XMLWriter::t_writecdata(const String& content) {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterWriteCDATA(m_ptr, (xmlChar*)content.data());
  }
  return ret != -1;
}

bool c_XMLWriter::t_endcdata() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndCDATA(m_ptr);
  }
  return ret != -1;
}

bool c_XMLWriter::t_startcomment() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterStartComment(m_ptr);
  }
  return ret != -1;
}

bool c_XMLWriter::t_writecomment(const String& content) {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterWriteComment(m_ptr, (xmlChar*)content.data());
  }
  return ret != -1;
}

bool c_XMLWriter::t_endcomment() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndComment(m_ptr);
  }
  return ret != -1;
}

bool c_XMLWriter::t_enddocument() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndDocument(m_ptr);
  }
  return ret != -1;
}

bool c_XMLWriter::t_startpi(const String& target) {
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

bool c_XMLWriter::t_writepi(const String& target, const String& content) {
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

bool c_XMLWriter::t_endpi() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndPI(m_ptr);
  }
  return ret != -1;
}

bool c_XMLWriter::t_text(const String& content) {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterWriteString(m_ptr, (xmlChar*)content.data());
  }
  return ret != -1;
}

bool c_XMLWriter::t_writeraw(const String& content) {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterWriteRaw(m_ptr, (xmlChar*)content.data());
  }
  return ret != -1;
}

bool c_XMLWriter::t_startdtd(const String& qualifiedname,
                             const String& publicid /* = null_string */,
                             const String& systemid /* = null_string */) {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterStartDTD(m_ptr, (xmlChar*)qualifiedname.data(),
                                xmls(publicid), xmls(systemid));
  }
  return ret != -1;
}

bool c_XMLWriter::t_writedtd(const String& name,
                             const String& publicid /* = null_string */,
                             const String& systemid /* = null_string */,
                             const String& subset /* = null_string */) {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterWriteDTD(m_ptr, (xmlChar*)name.data(),
                                xmls(publicid), xmls(systemid), xmls(subset));
  }
  return ret != -1;
}

bool c_XMLWriter::t_startdtdelement(const String& qualifiedname) {
  if (xmlValidateName((xmlChar*)qualifiedname.data(), 0)) {
    raise_warning("invalid element name: %s", qualifiedname.data());
    return false;
  }
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterStartDTDElement(m_ptr, (xmlChar*)qualifiedname.data());
  }
  return ret != -1;
}

bool c_XMLWriter::t_writedtdelement(const String& name, const String& content) {
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

bool c_XMLWriter::t_enddtdelement() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndDTDElement(m_ptr);
  }
  return ret != -1;
}

bool c_XMLWriter::t_startdtdattlist(const String& name) {
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

bool c_XMLWriter::t_writedtdattlist(const String& name, const String& content) {
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

bool c_XMLWriter::t_enddtdattlist() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndDTDAttlist(m_ptr);
  }
  return ret != -1;
}

bool c_XMLWriter::t_startdtdentity(const String& name, bool isparam) {
  if (xmlValidateName((xmlChar*)name.data(), 0)) {
    raise_warning("invalid attribute name: %s", name.data());
    return false;
  }
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterStartDTDEntity(m_ptr, isparam, (xmlChar*)name.data());
  }
  return ret != -1;
}

bool c_XMLWriter::t_writedtdentity(const String& name, const String& content,
                                   bool pe /* = false */,
                                   const String& publicid /* = null_string */,
                                   const String& systemid /* = null_string */,
                                   const String& ndataid /* = null_string */) {
  if (xmlValidateName((xmlChar*)name.data(), 0)) {
    raise_warning("invalid element name: %s", name.data());
    return false;
  }
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterWriteDTDEntity(m_ptr, pe, (xmlChar*)name.data(),
                                      xmls(publicid), xmls(systemid),
                                      xmls(ndataid), (xmlChar*)content.data());
  }
  return ret != -1;
}

bool c_XMLWriter::t_enddtdentity() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndDTDEntity(m_ptr);
  }
  return ret != -1;
}

bool c_XMLWriter::t_enddtd() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndDTD(m_ptr);
  }
  return ret != -1;
}

Variant c_XMLWriter::t_flush(bool empty /* = true */) {
  if (m_ptr && m_output) {
    xmlTextWriterFlush(m_ptr);
    String ret((char*)m_output->content, CopyString);
    if (empty) {
      xmlBufferEmpty(m_output);
    }
    return ret;
  }
  return "";
}

String c_XMLWriter::t_outputmemory(bool flush /* = true */) {
  return t_flush(flush);
}

///////////////////////////////////////////////////////////////////////////////
}
