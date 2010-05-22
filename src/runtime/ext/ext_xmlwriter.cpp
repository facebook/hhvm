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

#include <runtime/ext/ext_xmlwriter.h>

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(xmlwriter);
///////////////////////////////////////////////////////////////////////////////
// functions are just wrappers of object methods

Variant f_xmlwriter_open_memory() {
  c_xmlwriter *x = NEW(c_xmlwriter)();
  Object ret(x);
  if (x->t_openmemory()) {
    return ret;
  }
  return false;
}

Object f_xmlwriter_open_uri(CStrRef uri) {
  c_xmlwriter *x = NEW(c_xmlwriter)();
  Object ret(x);
  if (x->t_openuri(uri)) {
    return ret;
  }
  return false;
}

bool f_xmlwriter_set_indent_string(CObjRef xmlwriter, CStrRef indentstring) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_setindentstring(indentstring);
}

bool f_xmlwriter_set_indent(CObjRef xmlwriter, bool indent) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_setindent(indent);
}

bool f_xmlwriter_start_document(CObjRef xmlwriter,
                                CStrRef version /* = "1.0" */,
                                CStrRef encoding /* = null_string */,
                                CStrRef standalone /* = null_string */) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_startdocument(version, encoding, standalone);
}

bool f_xmlwriter_start_element(CObjRef xmlwriter, CStrRef name) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_startelement(name);
}

bool f_xmlwriter_start_element_ns(CObjRef xmlwriter, CStrRef prefix,
                                  CStrRef name, CStrRef uri) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_startelementns(prefix, name, uri);
}

bool f_xmlwriter_write_element_ns(CObjRef xmlwriter, CStrRef prefix,
                                  CStrRef name, CStrRef uri,
                                  CStrRef content /* = null_string */) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_writeelementns(prefix, name, uri, content);
}

bool f_xmlwriter_write_element(CObjRef xmlwriter, CStrRef name,
                               CStrRef content /* = null_string */) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_writeelement(name, content);
}

bool f_xmlwriter_end_element(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_endelement();
}

bool f_xmlwriter_full_end_element(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_fullendelement();
}

bool f_xmlwriter_start_attribute_ns(CObjRef xmlwriter, CStrRef prefix,
                                    CStrRef name, CStrRef uri) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_startattributens(prefix, name, uri);
}

bool f_xmlwriter_start_attribute(CObjRef xmlwriter, CStrRef name) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_startattribute(name);
}

bool f_xmlwriter_write_attribute_ns(CObjRef xmlwriter, CStrRef prefix,
                                    CStrRef name, CStrRef uri,
                                    CStrRef content) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_writeattributens(prefix, name, uri, content);
}

bool f_xmlwriter_write_attribute(CObjRef xmlwriter, CStrRef name,
                                 CStrRef value) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_writeattribute(name, value);
}

bool f_xmlwriter_end_attribute(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_endattribute();
}

bool f_xmlwriter_start_cdata(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_startcdata();
}

bool f_xmlwriter_write_cdata(CObjRef xmlwriter, CStrRef content) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_writecdata(content);
}

bool f_xmlwriter_end_cdata(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_endcdata();
}

bool f_xmlwriter_start_comment(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_startcomment();
}

bool f_xmlwriter_write_comment(CObjRef xmlwriter, CStrRef content) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_writecomment(content);
}

bool f_xmlwriter_end_comment(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_endcomment();
}

bool f_xmlwriter_end_document(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_enddocument();
}

bool f_xmlwriter_start_pi(CObjRef xmlwriter, CStrRef target) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_startpi(target);
}

bool f_xmlwriter_write_pi(CObjRef xmlwriter, CStrRef target, CStrRef content) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_writepi(target, content);
}

bool f_xmlwriter_end_pi(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_endpi();
}

bool f_xmlwriter_text(CObjRef xmlwriter, CStrRef content) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_text(content);
}

bool f_xmlwriter_write_raw(CObjRef xmlwriter, CStrRef content) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_writeraw(content);
}

bool f_xmlwriter_start_dtd(CObjRef xmlwriter, CStrRef qualifiedname,
                           CStrRef publicid /* = null_string */,
                           CStrRef systemid /* = null_string */) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_startdtd(qualifiedname, publicid, systemid);
}

bool f_xmlwriter_write_dtd(CObjRef xmlwriter, CStrRef name,
                           CStrRef publicid /* = null_string */,
                           CStrRef systemid /* = null_string */,
                           CStrRef subset /* = null_string */) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_writedtd(name, publicid, systemid, subset);
}

bool f_xmlwriter_start_dtd_element(CObjRef xmlwriter, CStrRef qualifiedname) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_startdtdelement(qualifiedname);
}

bool f_xmlwriter_write_dtd_element(CObjRef xmlwriter, CStrRef name,
                                   CStrRef content) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_writedtdelement(name, content);
}

bool f_xmlwriter_end_dtd_element(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_enddtdelement();
}

bool f_xmlwriter_start_dtd_attlist(CObjRef xmlwriter, CStrRef name) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_startdtdattlist(name);
}

bool f_xmlwriter_write_dtd_attlist(CObjRef xmlwriter, CStrRef name,
                                   CStrRef content) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_writedtdattlist(name, content);
}

bool f_xmlwriter_end_dtd_attlist(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_enddtdattlist();
}

bool f_xmlwriter_start_dtd_entity(CObjRef xmlwriter, CStrRef name,
                                  bool isparam) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_startdtdentity(name, isparam);
}

bool f_xmlwriter_write_dtd_entity(CObjRef xmlwriter, CStrRef name,
                                  CStrRef content, bool pe /* = false */,
                                  CStrRef publicid /* = null_string */,
                                  CStrRef systemid /* = null_string */,
                                  CStrRef ndataid /* = null_string */) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_writedtdentity(name, content);
}

bool f_xmlwriter_end_dtd_entity(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_enddtdentity();
}

bool f_xmlwriter_end_dtd(CObjRef xmlwriter) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_enddtd();
}

Variant f_xmlwriter_flush(CObjRef xmlwriter, bool empty /* = true */) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_flush(empty);
}

String f_xmlwriter_output_memory(CObjRef xmlwriter, bool flush /* = true */) {
  return xmlwriter.getTyped<c_xmlwriter>()->
    t_outputmemory(flush);
}

///////////////////////////////////////////////////////////////////////////////
// helpers

static int write_file(void *context, const char *buffer, int len) {
  return ((c_xmlwriter*)context)->m_uri->writeImpl(buffer, len);
}

static int close_file(void *context) {
  return 0;
}

static xmlChar *xmls(CStrRef s) {
  return s.isNull() ? NULL : (xmlChar*)s.data();
}

///////////////////////////////////////////////////////////////////////////////

c_xmlwriter::c_xmlwriter() : m_ptr(NULL), m_output(NULL), m_uri_output(NULL) {
}

c_xmlwriter::~c_xmlwriter() {
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

void c_xmlwriter::t___construct() {
}

bool c_xmlwriter::t_openmemory() {
  m_output = xmlBufferCreate();
  if (m_output == NULL) {
    raise_warning("Unable to create output buffer");
    return false;
  }
  return m_ptr = xmlNewTextWriterMemory(m_output, 0);
}

bool c_xmlwriter::t_openuri(CStrRef uri) {
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

bool c_xmlwriter::t_setindentstring(CStrRef indentstring) {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterSetIndentString(m_ptr, (xmlChar*)indentstring.data());
  }
  return ret != -1;
}

bool c_xmlwriter::t_setindent(bool indent) {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterSetIndent(m_ptr, indent);
  }
  return ret != -1;
}

bool c_xmlwriter::t_startdocument(CStrRef version /* = "1.0" */,
                                  CStrRef encoding /* = null_string */,
                                  CStrRef standalone /* = null_string */) {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterStartDocument(m_ptr, (const char *)xmls(version),
                                     (const char *)xmls(encoding),
                                     (const char *)xmls(standalone));
  }
  return ret != -1;
}

bool c_xmlwriter::t_startelement(CStrRef name) {
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

bool c_xmlwriter::t_startelementns(CStrRef prefix, CStrRef name, CStrRef uri) {
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

bool c_xmlwriter::t_writeelementns(CStrRef prefix, CStrRef name, CStrRef uri,
                                   CStrRef content /* = null_string */) {
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

bool c_xmlwriter::t_writeelement(CStrRef name,
                                 CStrRef content /* = null_string */) {
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

bool c_xmlwriter::t_endelement() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndElement(m_ptr);
  }
  return ret != -1;
}

bool c_xmlwriter::t_fullendelement() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterFullEndElement(m_ptr);
  }
  return ret != -1;
}

bool c_xmlwriter::t_startattributens(CStrRef prefix, CStrRef name,
                                     CStrRef uri) {
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

bool c_xmlwriter::t_startattribute(CStrRef name) {
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

bool c_xmlwriter::t_writeattributens(CStrRef prefix, CStrRef name, CStrRef uri,
                                     CStrRef content) {
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

bool c_xmlwriter::t_writeattribute(CStrRef name, CStrRef value) {
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

bool c_xmlwriter::t_endattribute() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndAttribute(m_ptr);
  }
  return ret != -1;
}

bool c_xmlwriter::t_startcdata() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterStartCDATA(m_ptr);
  }
  return ret != -1;
}

bool c_xmlwriter::t_writecdata(CStrRef content) {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterWriteCDATA(m_ptr, (xmlChar*)content.data());
  }
  return ret != -1;
}

bool c_xmlwriter::t_endcdata() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndCDATA(m_ptr);
  }
  return ret != -1;
}

bool c_xmlwriter::t_startcomment() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterStartComment(m_ptr);
  }
  return ret != -1;
}

bool c_xmlwriter::t_writecomment(CStrRef content) {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterWriteComment(m_ptr, (xmlChar*)content.data());
  }
  return ret != -1;
}

bool c_xmlwriter::t_endcomment() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndComment(m_ptr);
  }
  return ret != -1;
}

bool c_xmlwriter::t_enddocument() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndDocument(m_ptr);
  }
  return ret != -1;
}

bool c_xmlwriter::t_startpi(CStrRef target) {
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

bool c_xmlwriter::t_writepi(CStrRef target, CStrRef content) {
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

bool c_xmlwriter::t_endpi() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndPI(m_ptr);
  }
  return ret != -1;
}

bool c_xmlwriter::t_text(CStrRef content) {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterWriteString(m_ptr, (xmlChar*)content.data());
  }
  return ret != -1;
}

bool c_xmlwriter::t_writeraw(CStrRef content) {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterWriteRaw(m_ptr, (xmlChar*)content.data());
  }
  return ret != -1;
}

bool c_xmlwriter::t_startdtd(CStrRef qualifiedname,
                             CStrRef publicid /* = null_string */,
                             CStrRef systemid /* = null_string */) {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterStartDTD(m_ptr, (xmlChar*)qualifiedname.data(),
                                xmls(publicid), xmls(systemid));
  }
  return ret != -1;
}

bool c_xmlwriter::t_writedtd(CStrRef name,
                             CStrRef publicid /* = null_string */,
                             CStrRef systemid /* = null_string */,
                             CStrRef subset /* = null_string */) {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterWriteDTD(m_ptr, (xmlChar*)name.data(),
                                xmls(publicid), xmls(systemid), xmls(subset));
  }
  return ret != -1;
}

bool c_xmlwriter::t_startdtdelement(CStrRef qualifiedname) {
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

bool c_xmlwriter::t_writedtdelement(CStrRef name, CStrRef content) {
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

bool c_xmlwriter::t_enddtdelement() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndDTDElement(m_ptr);
  }
  return ret != -1;
}

bool c_xmlwriter::t_startdtdattlist(CStrRef name) {
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

bool c_xmlwriter::t_writedtdattlist(CStrRef name, CStrRef content) {
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

bool c_xmlwriter::t_enddtdattlist() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndDTDAttlist(m_ptr);
  }
  return ret != -1;
}

bool c_xmlwriter::t_startdtdentity(CStrRef name, bool isparam) {
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

bool c_xmlwriter::t_writedtdentity(CStrRef name, CStrRef content,
                                   bool pe /* = false */,
                                   CStrRef publicid /* = null_string */,
                                   CStrRef systemid /* = null_string */,
                                   CStrRef ndataid /* = null_string */) {
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

bool c_xmlwriter::t_enddtdentity() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndDTDEntity(m_ptr);
  }
  return ret != -1;
}

bool c_xmlwriter::t_enddtd() {
  int ret = -1;
  if (m_ptr) {
    ret = xmlTextWriterEndDTD(m_ptr);
  }
  return ret != -1;
}

Variant c_xmlwriter::t_flush(bool empty /* = true */) {
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

String c_xmlwriter::t_outputmemory(bool flush /* = true */) {
  return t_flush(flush);
}

Variant c_xmlwriter::t___destruct() {
  return null;
}

///////////////////////////////////////////////////////////////////////////////
}
