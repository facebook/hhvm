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

#include <runtime/ext/ext_xmlwriter.h>

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(xmlwriter);
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

Object f_xmlwriter_open_uri(CStrRef uri) {
  c_XMLWriter *x = NEWOBJ(c_XMLWriter)();
  Object ret(x);
  if (x->t_openuri(uri)) {
    return ret;
  }
  return NULL;
}

bool f_xmlwriter_set_indent_string(CObjRef xmlwriter, CStrRef indentstring) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_setindentstring(indentstring);
}

bool f_xmlwriter_set_indent(CObjRef xmlwriter, bool indent) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_setindent(indent);
}

bool f_xmlwriter_start_document(CObjRef xmlwriter,
                                CStrRef version /* = "1.0" */,
                                CStrRef encoding /* = null_string */,
                                CStrRef standalone /* = null_string */) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_startdocument(version, encoding, standalone);
}

bool f_xmlwriter_start_element(CObjRef xmlwriter, CStrRef name) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_startelement(name);
}

bool f_xmlwriter_start_element_ns(CObjRef xmlwriter, CStrRef prefix,
                                  CStrRef name, CStrRef uri) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_startelementns(prefix, name, uri);
}

bool f_xmlwriter_write_element_ns(CObjRef xmlwriter, CStrRef prefix,
                                  CStrRef name, CStrRef uri,
                                  CStrRef content /* = null_string */) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_writeelementns(prefix, name, uri, content);
}

bool f_xmlwriter_write_element(CObjRef xmlwriter, CStrRef name,
                               CStrRef content /* = null_string */) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_writeelement(name, content);
}

bool f_xmlwriter_end_element(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_endelement();
}

bool f_xmlwriter_full_end_element(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_fullendelement();
}

bool f_xmlwriter_start_attribute_ns(CObjRef xmlwriter, CStrRef prefix,
                                    CStrRef name, CStrRef uri) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_startattributens(prefix, name, uri);
}

bool f_xmlwriter_start_attribute(CObjRef xmlwriter, CStrRef name) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_startattribute(name);
}

bool f_xmlwriter_write_attribute_ns(CObjRef xmlwriter, CStrRef prefix,
                                    CStrRef name, CStrRef uri,
                                    CStrRef content) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_writeattributens(prefix, name, uri, content);
}

bool f_xmlwriter_write_attribute(CObjRef xmlwriter, CStrRef name,
                                 CStrRef value) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_writeattribute(name, value);
}

bool f_xmlwriter_end_attribute(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_endattribute();
}

bool f_xmlwriter_start_cdata(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_startcdata();
}

bool f_xmlwriter_write_cdata(CObjRef xmlwriter, CStrRef content) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_writecdata(content);
}

bool f_xmlwriter_end_cdata(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_endcdata();
}

bool f_xmlwriter_start_comment(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_startcomment();
}

bool f_xmlwriter_write_comment(CObjRef xmlwriter, CStrRef content) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_writecomment(content);
}

bool f_xmlwriter_end_comment(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_endcomment();
}

bool f_xmlwriter_end_document(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_enddocument();
}

bool f_xmlwriter_start_pi(CObjRef xmlwriter, CStrRef target) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_startpi(target);
}

bool f_xmlwriter_write_pi(CObjRef xmlwriter, CStrRef target, CStrRef content) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_writepi(target, content);
}

bool f_xmlwriter_end_pi(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_endpi();
}

bool f_xmlwriter_text(CObjRef xmlwriter, CStrRef content) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_text(content);
}

bool f_xmlwriter_write_raw(CObjRef xmlwriter, CStrRef content) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_writeraw(content);
}

bool f_xmlwriter_start_dtd(CObjRef xmlwriter, CStrRef qualifiedname,
                           CStrRef publicid /* = null_string */,
                           CStrRef systemid /* = null_string */) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_startdtd(qualifiedname, publicid, systemid);
}

bool f_xmlwriter_write_dtd(CObjRef xmlwriter, CStrRef name,
                           CStrRef publicid /* = null_string */,
                           CStrRef systemid /* = null_string */,
                           CStrRef subset /* = null_string */) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_writedtd(name, publicid, systemid, subset);
}

bool f_xmlwriter_start_dtd_element(CObjRef xmlwriter, CStrRef qualifiedname) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_startdtdelement(qualifiedname);
}

bool f_xmlwriter_write_dtd_element(CObjRef xmlwriter, CStrRef name,
                                   CStrRef content) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_writedtdelement(name, content);
}

bool f_xmlwriter_end_dtd_element(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_enddtdelement();
}

bool f_xmlwriter_start_dtd_attlist(CObjRef xmlwriter, CStrRef name) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_startdtdattlist(name);
}

bool f_xmlwriter_write_dtd_attlist(CObjRef xmlwriter, CStrRef name,
                                   CStrRef content) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_writedtdattlist(name, content);
}

bool f_xmlwriter_end_dtd_attlist(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_enddtdattlist();
}

bool f_xmlwriter_start_dtd_entity(CObjRef xmlwriter, CStrRef name,
                                  bool isparam) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_startdtdentity(name, isparam);
}

bool f_xmlwriter_write_dtd_entity(CObjRef xmlwriter, CStrRef name,
                                  CStrRef content, bool pe /* = false */,
                                  CStrRef publicid /* = null_string */,
                                  CStrRef systemid /* = null_string */,
                                  CStrRef ndataid /* = null_string */) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_writedtdentity(name, content);
}

bool f_xmlwriter_end_dtd_entity(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_enddtdentity();
}

bool f_xmlwriter_end_dtd(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_enddtd();
}

Variant f_xmlwriter_flush(CObjRef xmlwriter, bool empty /* = true */) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_flush(empty);
}

String f_xmlwriter_output_memory(CObjRef xmlwriter, bool flush /* = true */) {
  return xmlwriter.getTyped<c_XMLWriter>()->
    t_outputmemory(flush);
}

///////////////////////////////////////////////////////////////////////////////
// helpers

static int write_file(void *context, const char *buffer, int len) {
  return ((c_XMLWriter*)context)->m_uri->writeImpl(buffer, len);
}

static int close_file(void *context) {
  return 0;
}

static xmlChar *xmls(CStrRef s) {
  return s.isNull() ? NULL : (xmlChar*)s.data();
}

///////////////////////////////////////////////////////////////////////////////

c_XMLWriter::c_XMLWriter() : m_ptr(NULL), m_output(NULL), m_uri_output(NULL) {
}

c_XMLWriter::~c_XMLWriter() {
  if (m_ptr) {
    xmlFreeTextWriter(m_ptr);
  }
  if (m_output) {
    xmlBufferFree(m_output);
  }
  if (m_uri_output) {
    xmlOutputBufferClose(m_uri_output);
  }
}

void c_XMLWriter::t___construct() {
}

bool c_XMLWriter::t_openmemory() {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::openmemory);
  m_output = xmlBufferCreate();
  if (m_output == NULL) {
    raise_warning("Unable to create output buffer");
    return false;
  }
  return m_ptr = xmlNewTextWriterMemory(m_output, 0);
}

bool c_XMLWriter::t_openuri(CStrRef uri) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::openuri);
  Variant file = File::Open(uri, "wb");
  if (same(file, false)) {
    return false;
  }
  m_uri = file.toObject().getTyped<File>();

  m_uri_output = xmlOutputBufferCreateIO(write_file, close_file, this, NULL);
  if (m_uri_output == NULL) {
    raise_warning("Unable to create output buffer");
    return false;
  }
  m_ptr = xmlNewTextWriter(m_uri_output);
  return true;
}

bool c_XMLWriter::t_setindentstring(CStrRef indentstring) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::setindentstring);
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterSetIndentString(m_ptr, (xmlChar*)indentstring.data());
  }
  return ret != -1;
}

bool c_XMLWriter::t_setindent(bool indent) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::setindent);
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterSetIndent(m_ptr, indent);
  }
  return ret != -1;
}

bool c_XMLWriter::t_startdocument(CStrRef version /* = "1.0" */,
                                  CStrRef encoding /* = null_string */,
                                  CStrRef standalone /* = null_string */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::startdocument);
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterStartDocument(m_ptr, (const char *)xmls(version),
                                     (const char *)xmls(encoding),
                                     (const char *)xmls(standalone));
  }
  return ret != -1;
}

bool c_XMLWriter::t_startelement(CStrRef name) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::startelement);
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

bool c_XMLWriter::t_startelementns(CStrRef prefix, CStrRef name, CStrRef uri) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::startelementns);
  if (xmlValidateName((xmlChar*)name.data(), 0)) {
    raise_warning("invalid element name: %s", name.data());
    return false;
  }
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterStartElementNS(m_ptr, (xmlChar*)prefix.data(),
                                      (xmlChar*)name.data(),
                                      (xmlChar*)uri.data());
  }
  return ret != -1;
}

bool c_XMLWriter::t_writeelementns(CStrRef prefix, CStrRef name, CStrRef uri,
                                   CStrRef content /* = null_string */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::writeelementns);
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

bool c_XMLWriter::t_writeelement(CStrRef name,
                                 CStrRef content /* = null_string */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::writeelement);
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
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::endelement);
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndElement(m_ptr);
  }
  return ret != -1;
}

bool c_XMLWriter::t_fullendelement() {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::fullendelement);
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterFullEndElement(m_ptr);
  }
  return ret != -1;
}

bool c_XMLWriter::t_startattributens(CStrRef prefix, CStrRef name,
                                     CStrRef uri) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::startattributens);
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

bool c_XMLWriter::t_startattribute(CStrRef name) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::startattribute);
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

bool c_XMLWriter::t_writeattributens(CStrRef prefix, CStrRef name, CStrRef uri,
                                     CStrRef content) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::writeattributens);
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

bool c_XMLWriter::t_writeattribute(CStrRef name, CStrRef value) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::writeattribute);
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
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::endattribute);
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndAttribute(m_ptr);
  }
  return ret != -1;
}

bool c_XMLWriter::t_startcdata() {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::startcdata);
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterStartCDATA(m_ptr);
  }
  return ret != -1;
}

bool c_XMLWriter::t_writecdata(CStrRef content) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::writecdata);
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterWriteCDATA(m_ptr, (xmlChar*)content.data());
  }
  return ret != -1;
}

bool c_XMLWriter::t_endcdata() {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::endcdata);
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndCDATA(m_ptr);
  }
  return ret != -1;
}

bool c_XMLWriter::t_startcomment() {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::startcomment);
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterStartComment(m_ptr);
  }
  return ret != -1;
}

bool c_XMLWriter::t_writecomment(CStrRef content) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::writecomment);
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterWriteComment(m_ptr, (xmlChar*)content.data());
  }
  return ret != -1;
}

bool c_XMLWriter::t_endcomment() {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::endcomment);
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndComment(m_ptr);
  }
  return ret != -1;
}

bool c_XMLWriter::t_enddocument() {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::enddocument);
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndDocument(m_ptr);
  }
  return ret != -1;
}

bool c_XMLWriter::t_startpi(CStrRef target) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::startpi);
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

bool c_XMLWriter::t_writepi(CStrRef target, CStrRef content) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::writepi);
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
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::endpi);
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndPI(m_ptr);
  }
  return ret != -1;
}

bool c_XMLWriter::t_text(CStrRef content) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::text);
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterWriteString(m_ptr, (xmlChar*)content.data());
  }
  return ret != -1;
}

bool c_XMLWriter::t_writeraw(CStrRef content) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::writeraw);
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterWriteRaw(m_ptr, (xmlChar*)content.data());
  }
  return ret != -1;
}

bool c_XMLWriter::t_startdtd(CStrRef qualifiedname,
                             CStrRef publicid /* = null_string */,
                             CStrRef systemid /* = null_string */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::startdtd);
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterStartDTD(m_ptr, (xmlChar*)qualifiedname.data(),
                                xmls(publicid), xmls(systemid));
  }
  return ret != -1;
}

bool c_XMLWriter::t_writedtd(CStrRef name,
                             CStrRef publicid /* = null_string */,
                             CStrRef systemid /* = null_string */,
                             CStrRef subset /* = null_string */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::writedtd);
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterWriteDTD(m_ptr, (xmlChar*)name.data(),
                                xmls(publicid), xmls(systemid), xmls(subset));
  }
  return ret != -1;
}

bool c_XMLWriter::t_startdtdelement(CStrRef qualifiedname) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::startdtdelement);
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

bool c_XMLWriter::t_writedtdelement(CStrRef name, CStrRef content) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::writedtdelement);
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
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::enddtdelement);
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndDTDElement(m_ptr);
  }
  return ret != -1;
}

bool c_XMLWriter::t_startdtdattlist(CStrRef name) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::startdtdattlist);
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

bool c_XMLWriter::t_writedtdattlist(CStrRef name, CStrRef content) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::writedtdattlist);
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
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::enddtdattlist);
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndDTDAttlist(m_ptr);
  }
  return ret != -1;
}

bool c_XMLWriter::t_startdtdentity(CStrRef name, bool isparam) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::startdtdentity);
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

bool c_XMLWriter::t_writedtdentity(CStrRef name, CStrRef content,
                                   bool pe /* = false */,
                                   CStrRef publicid /* = null_string */,
                                   CStrRef systemid /* = null_string */,
                                   CStrRef ndataid /* = null_string */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::writedtdentity);
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
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::enddtdentity);
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndDTDEntity(m_ptr);
  }
  return ret != -1;
}

bool c_XMLWriter::t_enddtd() {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::enddtd);
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndDTD(m_ptr);
  }
  return ret != -1;
}

Variant c_XMLWriter::t_flush(bool empty /* = true */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::flush);
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
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::outputmemory);
  return t_flush(flush);
}

Variant c_XMLWriter::t___destruct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(XMLWriter, XMLWriter::__destruct);
  return null;
}

///////////////////////////////////////////////////////////////////////////////
}
