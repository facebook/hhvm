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

#ifndef incl_HPHP_EXT_LIBXML_H_
#define incl_HPHP_EXT_LIBXML_H_

#include "hphp/runtime/ext/extension.h"

#include <libxml/parser.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool libxml_use_internal_error();
void libxml_add_error(const std::string& msg);
String libxml_get_valid_file_path(const String& source);
String libxml_get_valid_file_path(const char* source);

int libxml_streams_IO_read(void* context, char* buffer, int len);
int libxml_streams_IO_write(void* context, const char* buffer, int len);
int libxml_streams_IO_close(void* context);
int libxml_streams_IO_nop_close(void* context);

void php_libxml_node_free(xmlNodePtr node);
void php_libxml_node_free_resource(xmlNodePtr node);

bool HHVM_FUNCTION(libxml_disable_entity_loader, bool disable = true);

/*
 * LibXML resource wrappers.
 *
 * Several extensions use libxml (DOMDocument, SimpleXML, ext_xsl) as their
 * underlying representation of an XML DOM. Because these extensions share
 * libxml data we use Resource wrappers to control destruction of libxml data.
 *
 * In addition to their req::ptr behavior these wrappers also serve as a
 * cache for DOMNode objects that have been associated with particular xmlNodes
 * and retain information about the owning document for each node.
 *
 * Unfortunately (read: Because, PHP), these wrappers will also sometimes serve
 * as weak-references. In particular it is possible for (1) the cached
 * DOMNode object to be free'd before the xmlNode*, in which case the DOMNode
 * is responsible for clearing the cache, and (2) for the underlying xmlNode*
 * to be free'd while there are still req::ptr holding XMLNodeData resouces.
 *
 * In the event of (1) the only reference to the free'd object will be the
 * object in m_cache, and as such an raw ObjectData* is used as a quasi-weak
 * pointer.
 *
 * In PHP, if the root of an orphaned sub-tree of a Document, or of an orphaned
 * subtree with no associated document goes out of scope all of its descendants
 * are freed. When this is done any live XMLNodeDatas remain valid, however,
 * their xmlNode* pointers are cleared to indicate the bound node no longer
 * exists.
 *
 * Any node properly connected to the root element of an xmlDoc* will remain
 * valid until such time the the owning xmlDoc* becomes invalid or the node
 * becomes orphaned from the root and its subtree is freed.
 *
 * Documents will remain valid until such time that there are no further
 * references to nodes contained therein (including references to nodes whose
 * underlying representation has been freed).
 *
 * The libxml_register_node() function will "Do the right thing" (tm) when
 * given an xmlNodePtr.  Specifically, if the node already has an associated
 * XMLNodeData or XMLDocumnentData, that resource is attached- otherwise a new
 * resource is created. Additionally, if a new resource is created for a node
 * which itself is attached to a document with no associated resource, a
 * resource will be created for that document.
 *
 * It is not necessary to cast xmlNode* to xmlDoc* before passing these pointers
 * to libxml_register_node(), the type field will always be inspected before
 * creating a new node resource.
 *
 * These resource classes are based on the PHP php_libxml_node_object
 * (XMLNodeData), and php_libxml_ref_obj (XMLDocumentData). Rather than track
 * these classes separately we track them as a single unified set of XMLNodeData
 * resources.
 *
 * https://github.com/php/php-src/blob/master/ext/libxml/php_libxml.h
 */

struct XMLDocumentData;

struct XMLNodeData : SweepableResourceData {
  DECLARE_RESOURCE_ALLOCATION(XMLNodeData)

  explicit XMLNodeData(xmlNodePtr p);
  virtual ~XMLNodeData();

  ObjectData* getCache() const { return m_cache; }
  void clearCache() { m_cache = nullptr; }
  void setCache(ObjectData* o) { m_cache = o; }

  void reset() { m_node = nullptr; }
  void setDoc(req::ptr<XMLDocumentData>&& doc);

  xmlDocPtr docp() const;
  xmlNodePtr nodep() const { return m_node; }
  req::ptr<XMLDocumentData> doc();
  void unlink() { xmlUnlinkNode(m_node); }

private:
  ObjectData* m_cache {nullptr}; // XXX: to avoid a cycle this is a weak ref
  xmlNodePtr m_node {nullptr};
  req::ptr<XMLDocumentData> m_doc {nullptr};

  friend class XMLDocumentData;
};

struct XMLDocumentData : XMLNodeData {
  DECLARE_RESOURCE_ALLOCATION(XMLDocumentData)

  explicit XMLDocumentData(xmlDocPtr p)
    : XMLNodeData((xmlNodePtr)p)
    , m_formatoutput(false)
    , m_validateonparse(false)
    , m_resolveexternals(false)
    , m_preservewhitespace(true)
    , m_substituteentities(false)
    , m_stricterror(true)
    , m_recover(false)
    , m_destruct(false)
  {
    assert(p->type == XML_HTML_DOCUMENT_NODE || p->type == XML_DOCUMENT_NODE);
  }

  xmlDocPtr docp() const { return (xmlDocPtr)m_node; }
  void attachNode() { m_liveNodes++; }
  void detachNode() {
    assert(m_liveNodes);
    if (!--m_liveNodes && m_destruct) cleanup();
  }

  void cleanup();
  ~XMLDocumentData() override { cleanup(); }

  Array m_classmap;
  uint32_t m_liveNodes {0};

  unsigned m_formatoutput       :1;
  unsigned m_validateonparse    :1;
  unsigned m_resolveexternals   :1;
  unsigned m_preservewhitespace :1;
  unsigned m_substituteentities :1;
  unsigned m_stricterror        :1;
  unsigned m_recover            :1;
  unsigned m_destruct           :1; // cleanup when last node de-registers
};

using XMLNode = req::ptr<XMLNodeData>;

inline XMLNode libxml_register_node(xmlNodePtr p) {
  if (!p) return nullptr;
  if (p->_private) {
    return XMLNode(reinterpret_cast<XMLNodeData*>(p->_private));
  }

  if (p->type == XML_HTML_DOCUMENT_NODE ||
      p->type == XML_DOCUMENT_NODE) {
    assert(p->doc == (xmlDocPtr)p);

    return req::make<XMLDocumentData>((xmlDocPtr)p);
  }
  return req::make<XMLNodeData>(p);
}

inline XMLNode libxml_register_node(xmlDocPtr p) {
  return libxml_register_node((xmlNodePtr)p);
}


inline XMLNodeData::XMLNodeData(xmlNodePtr p) : m_node(p) {
  assert(p && !p->_private);
  m_node->_private = this;

  if (p->doc && p != (xmlNodePtr)p->doc) {
    m_doc = libxml_register_node((xmlNodePtr)p->doc)->doc();
    m_doc->attachNode();
  }
}

inline XMLNodeData::~XMLNodeData() {
  if (m_node) {
    assert(!m_cache && m_node->_private == this);

    m_node->_private = nullptr;
    php_libxml_node_free_resource(m_node);
  }
  if (m_doc) m_doc->detachNode();
}

inline void XMLNodeData::setDoc(req::ptr<XMLDocumentData>&& doc) {
  if (m_doc) m_doc->detachNode();
  if (doc) doc->attachNode();
  m_doc = std::move(doc);
}

inline req::ptr<XMLDocumentData> XMLNodeData::doc() {
  if (!m_node) return nullptr;

  if (m_node->type == XML_HTML_DOCUMENT_NODE ||
      m_node->type == XML_DOCUMENT_NODE) {
    return req::ptr<XMLDocumentData>(static_cast<XMLDocumentData*>(this));
  }

  if (!m_doc) {
    assert(!m_node->doc);
    return nullptr;
  }

  assert(m_doc.get() == libxml_register_node((xmlNodePtr)m_node->doc).get());
  return m_doc;
}

inline xmlDocPtr XMLNodeData::docp() const {
  auto docData = const_cast<XMLNodeData*>(this)->doc();
  return docData ? docData->docp() : nullptr;
}

#define LIBXML_SAVE_NOEMPTYTAG 1<<2

///////////////////////////////////////////////////////////////////////////////
}
#endif
