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

#include "hphp/runtime/base/base-includes.h"

#include <memory>

#include "hphp/runtime/ext/domdocument/ext_domdocument_includes.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/vm/native-prop-handler.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(dom_import_simplexml,
                      const Object& node);

Variant php_dom_create_object(xmlNodePtr obj, Object doc, bool owner = false);

template <class T>
T* toDOMNativeData(ObjectData* obj) {
  if (!obj->instanceof(T::getClass())) {
    throw_invalid_object_type(obj->getClassName().data());
  }
  return Native::data<T>(obj);
}

///////////////////////////////////////////////////////////////////////////////
// class DOMNode

class DOMDocument;

class DOMNode {
public:
  DOMNode() : m_node(nullptr) {}
  virtual ~DOMNode() {}
  DOMNode& operator=(const DOMNode& src) {
    m_doc = src.doc();
    m_node = src.m_node;
    return *this;
  }
  virtual Object doc() const {
    return m_doc;
  }
  virtual DOMDocument* doc_data() {
    ObjectData* data = doc().get();
    return data == nullptr ? nullptr : Native::data<DOMDocument>(data);
  }
  static Class* getClass();
  static Object newInstance(Object doc, xmlNodePtr attrp);

  Object m_doc;
  xmlNodePtr m_node;

  static Class* c_Class;
  static const StaticString c_ClassName;
protected:
  static Object newInstance(Class* subclass, Object doc, xmlNodePtr attrp);
};

DOMNode* toDOMNode(ObjectData* obj);

///////////////////////////////////////////////////////////////////////////////
// class DOMAttr

class DOMAttr : public DOMNode {
public:
  static Class* getClass();
  static Object newInstance(Object doc, xmlNodePtr attrp);

  static Class* c_Class;
  static const StaticString c_ClassName;
};

///////////////////////////////////////////////////////////////////////////////
// class DOMCharacterData

class DOMCharacterData : public DOMNode {
public:
  static Class* c_Class;
  static const StaticString c_ClassName;
};

///////////////////////////////////////////////////////////////////////////////
// class DOMComment

class DOMComment : public DOMCharacterData {
public:
  static Class* getClass();
  static Object newInstance(Object doc, xmlNodePtr attrp);

  static Class* c_Class;
  static const StaticString c_ClassName;
};

///////////////////////////////////////////////////////////////////////////////
// class DOMText

class DOMText : public DOMCharacterData {
public:
  static Class* getClass();
  static Object newInstance(Object doc, xmlNodePtr attrp);

  static Class* c_Class;
  static const StaticString c_ClassName;
};

///////////////////////////////////////////////////////////////////////////////
// class DOMCdataSection

class DOMCdataSection : public DOMText {
public:
  static Class* getClass();
  static Object newInstance(Object doc, xmlNodePtr attrp);

  static Class* c_Class;
  static const StaticString c_ClassName;
};

///////////////////////////////////////////////////////////////////////////////
// class DOMDocument

typedef hphp_hash_set<xmlNodePtr, pointer_hash<xmlNode>> XmlNodeSet;

class DOMDocument : public DOMNode {
public:
  DOMDocument();
  virtual ~DOMDocument();
  DOMDocument& operator=(const DOMDocument& src) {
    DOMNode::operator=(src);
    return *this;
  }
  void sweep();
  virtual Object doc() const {
    // m_self is null if it is not an owner
    return m_owner ? Object(m_self) : DOMNode::doc();
  }
  virtual DOMDocument* doc_data() {
    return this;
  }
  Variant save_html_or_xml(bool as_xml, const Object& node = null_object);
  static Object newInstance();
  static Class* getClass();

  bool m_formatoutput;
  bool m_validateonparse;
  bool m_resolveexternals;
  bool m_preservewhitespace;
  bool m_substituteentities;
  bool m_stricterror;
  bool m_recover;
  Array m_classmap;
  std::unique_ptr<XmlNodeSet> m_orphans;
  bool m_owner;
  ObjectData* m_self;

  static Class* c_Class;
  static const StaticString c_ClassName;
};

///////////////////////////////////////////////////////////////////////////////
// class DOMDocumentFragment

class DOMDocumentFragment : public DOMNode {
public:
  static Class* getClass();
  static Object newInstance(Object doc, xmlNodePtr attrp);

  static Class* c_Class;
  static const StaticString c_ClassName;
};

///////////////////////////////////////////////////////////////////////////////
// class DOMDocumentType

class DOMDocumentType : public DOMNode {
public:
  static Class* getClass();

  static Class* c_Class;
  static const StaticString c_ClassName;
};

///////////////////////////////////////////////////////////////////////////////
// class DOMElement

class DOMElement : public DOMNode {
public:
  // Allow serialization, but no native data is actually serialized.
  Variant sleep() const {
    return init_null();
  }
  void wakeup(const Variant&, ObjectData*) {}
  static Class* getClass();
  static Object newInstance(Object doc, xmlNodePtr attrp);

  static Class* c_Class;
  static const StaticString c_ClassName;
};

///////////////////////////////////////////////////////////////////////////////
// class DOMEntity

class DOMEntity : public DOMNode {
public:
  static Class* getClass();
  static Object newInstance(Object doc, xmlNodePtr attrp);

  static Class* c_Class;
  static const StaticString c_ClassName;
};

///////////////////////////////////////////////////////////////////////////////
// class DOMEntityReference

class DOMEntityReference : public DOMNode {
public:
  static Class* getClass();

  static Class* c_Class;
  static const StaticString c_ClassName;
};

///////////////////////////////////////////////////////////////////////////////
// class DOMNotation

class DOMNotation : public DOMNode {
public:
  static Class* getClass();

  static Class* c_Class;
  static const StaticString c_ClassName;
};

///////////////////////////////////////////////////////////////////////////////
// class DOMProcessingInstruction

class DOMProcessingInstruction : public DOMNode {
public:
  static Class* getClass();
  static Object newInstance(Object doc, xmlNodePtr attrp);

  static Class* c_Class;
  static const StaticString c_ClassName;
};

///////////////////////////////////////////////////////////////////////////////
// class DOMNameSpaceNode

class DOMNameSpaceNode : public DOMNode {
public:
  static Class* getClass();

  static Class* c_Class;
  static const StaticString c_ClassName;
};



///////////////////////////////////////////////////////////////////////////////


// base class of DOMNodeList and DOMNamedNodeMap
class dom_iterable {
public:
  dom_iterable() : m_ht(nullptr) {}
  DOMNode* getBaseNodeData() {
    return toDOMNode(m_baseobj.get());
  }

  Object m_doc;
  Object m_baseobj;
  int m_nodetype;
  xmlHashTable *m_ht;
  String m_local;
  String m_ns;
  Array m_baseobjptr;
};

///////////////////////////////////////////////////////////////////////////////
// class DOMNodeIterator

class DOMNodeIterator : public Sweepable {
public:
  DOMNodeIterator() : m_objmap(nullptr), m_iter(), m_index(-1) {}
  ~DOMNodeIterator() {
    sweep();
  }
  virtual void sweep() {}
  void reset_iterator();
  void set_iterator(ObjectData* o, dom_iterable *objmap);
  void setKeyIsNamed() {
    m_keyIsNamed = true;
  }
  static Class* getClass();

  Object m_o;
  dom_iterable *m_objmap;
  ArrayIter m_iter;
  int m_index;
  bool m_keyIsNamed = false;
  Object m_curobj;

  static Class* c_Class;
  static const StaticString c_ClassName;
};

///////////////////////////////////////////////////////////////////////////////
// class DOMNamedNodeMap

class DOMNamedNodeMap : public dom_iterable {
public:
  static Class* getClass();
  static Object newInstance(Object doc, Object base, int node_type,
                            xmlHashTable* ht = nullptr);

  static Class* c_Class;
  static const StaticString c_ClassName;
};

///////////////////////////////////////////////////////////////////////////////
// class DOMNodeList

class DOMNodeList : public dom_iterable {
public:
  static Class* getClass();
  static Object newInstance(Object doc, Object base, int node_type,
                            String local = String(), String ns = String());

  static Class* c_Class;
  static const StaticString c_ClassName;
};

///////////////////////////////////////////////////////////////////////////////
// class DOMImplementation

class DOMImplementation {
public:
  static Class* getClass();

  static Class* c_Class;
  static const StaticString c_ClassName;
};

///////////////////////////////////////////////////////////////////////////////
// class DOMXPath

class DOMXPath : public Sweepable {
public:
  DOMXPath() : m_node(nullptr), m_registerPhpFunctions(0) {}
  ~DOMXPath() {
    sweep();
  }
  virtual void sweep();
  static Class* getClass();

  xmlNodePtr m_node;
  Object m_doc;
  Array m_node_list;
  int m_registerPhpFunctions;
  Array m_registered_phpfunctions;

  static Class* c_Class;
  static const StaticString c_ClassName;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_DOMDOCUMENT_H_
