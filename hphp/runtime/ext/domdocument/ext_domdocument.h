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

#ifndef incl_HPHP_EXT_DOMDOCUMENT_H_
#define incl_HPHP_EXT_DOMDOCUMENT_H_

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/ext/extension.h"

#include <memory>

#include "hphp/runtime/ext/domdocument/ext_domdocument_includes.h"
#include "hphp/runtime/ext/libxml/ext_libxml.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/vm/native-prop-handler.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(dom_import_simplexml,
                      const Object& node);

Variant php_dom_create_object(xmlNodePtr obj, req::ptr<XMLDocumentData> doc);

Object newDOMDocument(bool construct = true);

///////////////////////////////////////////////////////////////////////////////
// class DOMNode

struct DOMNode {
  virtual ~DOMNode() {
    if (m_node) {
      assert(m_node->getCache() &&
             Native::data<DOMNode>(m_node->getCache()) == this);
      m_node->clearCache();
    }
  }
  req::ptr<XMLDocumentData> doc() const { return m_node->doc(); }
  XMLNode node() const { return m_node; }
  xmlNodePtr nodep() const {
    return m_node ? m_node->nodep() : nullptr;
  }

  void setDoc(req::ptr<XMLDocumentData>&& doc) {
    assert(m_node);
    m_node->setDoc(std::move(doc));
  }

  void setNode(XMLNode n) {
    if (m_node) {
      assert(m_node->getCache() &&
             Native::data<DOMNode>(m_node->getCache()) == this);
      m_node->clearCache();
    }

    m_node = n;
    m_node->setCache(toObject());
  }

  void setNode(xmlNodePtr n) { setNode(libxml_register_node(n)); }

private:
  template <typename F> friend void scan(const HPHP::DOMNode&, F&);
  ObjectData* toObject() {
    return reinterpret_cast<ObjectData*>(this + 1);
  }
  XMLNode m_node {nullptr};
};

Variant save_html_or_xml(DOMNode* obj, bool as_xml,
                         const Object& node = null_object);

///////////////////////////////////////////////////////////////////////////////
// class DOMElement

struct DOMElement : DOMNode {
  // Allow serialization, but no native data is actually serialized.
  Variant sleep() const { return init_null(); }
  void wakeup(const Variant&, ObjectData*) {}
};

///////////////////////////////////////////////////////////////////////////////

struct DOMIterable {
  DOMNode* getBaseNodeData() {
    if (!m_baseobj) {
      throw_null_pointer_exception();
    }

    return Native::data<DOMNode>(m_baseobj);
  }

  req::ptr<XMLDocumentData> m_doc {nullptr};
  Object m_baseobj;
  int m_nodetype;
  xmlHashTable* m_ht {nullptr};
  String m_local;
  String m_ns;
  Array m_baseobjptr;
};

///////////////////////////////////////////////////////////////////////////////
// class DOMNodeIterator

struct DOMNodeIterator {
  void reset_iterator();
  void set_iterator(ObjectData* o, DOMIterable* objmap);
  void setKeyIsNamed() { m_keyIsNamed = true; }

  Object m_o;
  Object m_curobj;
  DOMIterable* m_objmap {nullptr};
  ArrayIter m_iter;
  int m_index {-1};
  bool m_keyIsNamed {false};
};

///////////////////////////////////////////////////////////////////////////////
// class DOMXPath

struct DOMXPath {
  ~DOMXPath() { sweep(); }
  void sweep();
  xmlXPathContextPtr m_node {nullptr};
  Object m_doc;
  Array m_node_list;
  int m_registerPhpFunctions {0};
  Array m_registered_phpfunctions;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_DOMDOCUMENT_H_
