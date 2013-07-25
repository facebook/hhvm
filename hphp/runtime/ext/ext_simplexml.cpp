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

#include "hphp/runtime/ext/ext_simplexml.h"
#include "hphp/runtime/ext/ext_file.h"
#include "hphp/runtime/ext/ext_class.h"
#include "hphp/runtime/ext/ext_domdocument.h"
#include "hphp/runtime/base/class_info.h"
#include "hphp/runtime/base/request_local.h"
#include "hphp/system/systemlib.h"

#ifndef LIBXML2_NEW_BUFFER
# define xmlOutputBufferGetSize(buf)    ((buf)->buffer->use)
# define xmlOutputBufferGetContent(buf) ((buf)->buffer->content)
#endif

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(SimpleXML);
///////////////////////////////////////////////////////////////////////////////

// This is to make sure each node holds one reference of m_doc, so not to let
// it go out of scope.
class XmlDocWrapper : public SweepableResourceData {
public:
  DECLARE_OBJECT_ALLOCATION_NO_SWEEP(XmlDocWrapper)

  static StaticString s_class_name;
  // overriding ResourceData
  virtual CStrRef o_getClassNameHook() const { return s_class_name; }

  XmlDocWrapper(xmlDocPtr doc, CStrRef cls, Object domNode = nullptr)
    : m_doc(doc), m_cls(cls), m_domNode(domNode) {
    if (!domNode.isNull()) {
      DEBUG_ONLY c_DOMNode *domnode = domNode.getTyped<c_DOMNode>();
      assert(!domnode || domnode->m_node == (xmlNodePtr) doc);
    }
  }

  CStrRef getClass() { return m_cls; }

  void sweep() {
    // if m_domNode isn't null, then he owns the m_doc. Otherwise, I own it
    if (m_doc && m_domNode.isNull()) {
      xmlFreeDoc(m_doc);
    }
  }
  ~XmlDocWrapper() { XmlDocWrapper::sweep(); }
private:
  xmlDocPtr m_doc;
  String m_cls;
  // Hold onto the original owner of the doc so it doesn't get free()d.
  Object m_domNode;
};
IMPLEMENT_OBJECT_ALLOCATION_NO_DEFAULT_SWEEP(XmlDocWrapper)

StaticString XmlDocWrapper::s_class_name("xmlDoc");

///////////////////////////////////////////////////////////////////////////////
// helpers

static inline bool match_ns(xmlNodePtr node, CStrRef ns, bool is_prefix) {
  if (ns.empty()) {
    return true;
  }
  if (node->ns == NULL || node->ns->prefix == NULL) {
    return false;
  }
  if (node->ns && !xmlStrcmp(is_prefix ? node->ns->prefix : node->ns->href,
                             (const xmlChar *)ns.data())) {
    return true;
  }
  return false;
}

static String node_list_to_string(xmlDocPtr doc, xmlNodePtr list) {
  xmlChar *tmp = xmlNodeListGetString(doc, list, 1);
  String res((char*) tmp, CopyString);
  xmlFree(tmp);
  return res;
}

static Array collect_attributes(xmlNodePtr node, CStrRef ns, bool is_prefix) {
  assert(node);
  Array attributes = Array::Create();
  if (node->type != XML_ENTITY_DECL) {
    for (xmlAttrPtr attr = node->properties; attr; attr = attr->next) {
      if (match_ns((xmlNodePtr)attr, ns, is_prefix)) {
        String n = String((char*)attr->name, xmlStrlen(attr->name), CopyString);
        attributes.set(n, node_list_to_string(node->doc, attr->children));
      }
    }
  }
  return attributes;
}

static void add_property(Array &properties, xmlNodePtr node, Object value) {
  const char *name = (char *)node->name;
  if (name) {
    int namelen = xmlStrlen(node->name);
    String sname(name, namelen, CopyString);

    if (properties.exists(sname)) {
      Variant &existing = properties.lval(sname);
      if (existing.is(KindOfArray)) {
        existing.append(value);
      } else {
        Array newdata;
        newdata.append(existing);
        newdata.append(value);
        properties.set(sname, newdata);
      }
    } else {
      properties.set(sname, value);
    }
  }
}

static Object create_text(CResRef doc, xmlNodePtr node,
                          CStrRef value, CStrRef ns,
                          bool is_prefix, bool free_text) {
  Object obj = create_object(doc.getTyped<XmlDocWrapper>()->
                             getClass(), Array(), false);
  c_SimpleXMLElement *elem = obj.getTyped<c_SimpleXMLElement>();
  elem->m_doc = doc;
  elem->m_node = node->parent; // assign to parent, not node
  elem->m_children.set(0, value);
  elem->m_is_text = true;
  elem->m_free_text = free_text;
  elem->m_attributes = collect_attributes(node->parent, ns, is_prefix);
  return obj;
}

static Array create_children(CResRef doc, xmlNodePtr root,
                             CStrRef ns, bool is_prefix);

static Object create_element(CResRef doc, xmlNodePtr node,
                             CStrRef ns, bool is_prefix) {
  Object obj = create_object(doc.getTyped<XmlDocWrapper>()->
                             getClass(), Array(), false);
  c_SimpleXMLElement *elem = obj.getTyped<c_SimpleXMLElement>();
  elem->m_doc = doc;
  elem->m_node = node;
  if (node) {
    elem->m_children = create_children(doc, node, ns, is_prefix);
    elem->m_attributes = collect_attributes(node, ns, is_prefix);
  }
  return obj;
}

static Array create_children(CResRef doc, xmlNodePtr root,
                             CStrRef ns, bool is_prefix) {
  Array properties = Array::Create();
  for (xmlNodePtr node = root->children; node; node = node->next) {
    if (root->parent && root->parent->type == XML_DOCUMENT_NODE &&
        node->type == XML_COMMENT_NODE) continue;
    if (node->children || node->prev || node->next) {
      if (node->type == XML_TEXT_NODE) {
        // bad node from parser, ignoring it...
        continue;
      }
    } else {
      if (node->type == XML_TEXT_NODE) {
        if (node->content && *node->content) {
          add_property
            (properties, root,
             create_text(doc, node, node_list_to_string(root->doc, node),
                         ns, is_prefix, true));
        }
        continue;
      }
    }

    if (node->type != XML_ELEMENT_NODE || match_ns(node, ns, is_prefix)) {
      xmlNodePtr child = node->children;
      Object sub;
      if (child && child->type == XML_TEXT_NODE && !xmlIsBlankNode(child)) {
        sub = create_text(doc, child, node_list_to_string(root->doc, child),
                          ns, is_prefix, false);
      } else {
        sub = create_element(doc, node, ns, is_prefix);
      }
      add_property(properties, node, sub);
    }
  }
  return properties;
}

static inline void add_namespace_name(Array &out, xmlNsPtr ns) {
  String prefix = ns->prefix ? String((const char*)ns->prefix) :
                               String(empty_string);
  if (!out.exists(prefix)) {
    out.set(prefix, String((char*)ns->href, CopyString));
  }
}

static void add_namespaces(Array &out, xmlNodePtr node, bool recursive) {
  if (node->ns) {
    add_namespace_name(out, node->ns);
  }

  for (xmlAttrPtr attr = node->properties; attr; attr = attr->next) {
    if (attr->ns) {
      add_namespace_name(out, attr->ns);
    }
  }

  if (recursive) {
    for (node = node->children; node; node = node->next) {
      if (node->type == XML_ELEMENT_NODE) {
        add_namespaces(out, node, true);
      }
    }
  }
}

static void add_registered_namespaces(Array &out, xmlNodePtr node,
                                      bool recursive) {
  if (node->type == XML_ELEMENT_NODE) {
    for (xmlNsPtr ns = node->nsDef; ns; ns = ns->next) {
      add_namespace_name(out, ns);
    }
    if (recursive) {
      for (node = node->children; node; node = node->next) {
        add_registered_namespaces(out, node, true);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// simplexml

Variant f_simplexml_import_dom(CObjRef node,
                               CStrRef class_name /* = "SimpleXMLElement" */) {

  c_DOMNode *domnode = node.getTyped<c_DOMNode>();
  xmlNodePtr nodep = domnode->m_node;

  if (nodep) {
    if (nodep->doc == nullptr) {
      raise_warning("Imported Node must have associated Document");
      return uninit_null();
    }
    if (nodep->type == XML_DOCUMENT_NODE ||
        nodep->type == XML_HTML_DOCUMENT_NODE) {
      nodep = xmlDocGetRootElement((xmlDocPtr) nodep);
    }
  }

  if (nodep && nodep->type == XML_ELEMENT_NODE) {
    Resource obj =
      Resource(NEWOBJ(XmlDocWrapper)(nodep->doc, class_name, node));
    return create_element(obj, nodep, String(), false);
  } else {
    raise_warning("Invalid Nodetype to import");
    return uninit_null();
  }
}

Variant f_simplexml_load_string(CStrRef data,
                                CStrRef class_name /* = "SimpleXMLElement" */,
                                int64_t options /* = 0 */,
                                CStrRef ns /* = "" */,
                                bool is_prefix /* = false */) {
  Class* cls;
  if (!class_name.empty()) {
    cls = Unit::loadClass(class_name.get());
    if (!cls) {
      throw_invalid_argument("class not found: %s", class_name.data());
      return uninit_null();
    }
    if (!cls->classof(c_SimpleXMLElement::s_cls)) {
      throw_invalid_argument(
        "simplexml_load_string() expects parameter 2 to be a class name "
        "derived from SimpleXMLElement, '%s' given",
        class_name.data());
      return uninit_null();
    }
  } else {
    cls = c_SimpleXMLElement::s_cls;
  }

  xmlDocPtr doc = xmlReadMemory(data.data(), data.size(), NULL, NULL, options);
  xmlNodePtr root = xmlDocGetRootElement(doc);
  if (!doc) {
    return false;
  }

  return create_element(Resource(NEWOBJ(XmlDocWrapper)(doc, cls->nameRef())),
                                           root, ns, is_prefix);
}

Variant f_simplexml_load_file(CStrRef filename,
                              CStrRef class_name /* = "SimpleXMLElement" */,
                              int64_t options /* = 0 */, CStrRef ns /* = "" */,
                              bool is_prefix /* = false */) {
  String str = f_file_get_contents(filename);
  return f_simplexml_load_string(str, class_name, options, ns, is_prefix);
}

///////////////////////////////////////////////////////////////////////////////
// SimpleXMLElement

c_SimpleXMLElement::c_SimpleXMLElement(Class* cb) :
    ExtObjectDataFlags<ObjectData::UseGet|
                       ObjectData::UseSet|
                       ObjectData::UseIsset|
                       ObjectData::UseUnset|
                       ObjectData::CallToImpl>(cb),
      m_node(NULL), m_is_text(false), m_free_text(false),
      m_is_attribute(false), m_is_children(false), m_is_property(false),
      m_xpath(NULL) {
  m_children = Array::Create();
}

c_SimpleXMLElement::~c_SimpleXMLElement() {
  c_SimpleXMLElement::sweep();
}

void c_SimpleXMLElement::sweep() {
  if (m_xpath) {
    xmlXPathFreeContext(m_xpath);
  }
}

void c_SimpleXMLElement::t___construct(CStrRef data, int64_t options /* = 0 */,
                                       bool data_is_url /* = false */,
                                       CStrRef ns /* = "" */,
                                       bool is_prefix /* = false */) {
  String xml = data;
  if (data_is_url) {
    Variant ret = f_file_get_contents(data);
    if (same(ret, false)) {
      raise_warning("Unable to retrieve XML content from %s", data.data());
      return;
    }
    xml = ret.toString();
  }

  xmlDocPtr doc = xmlReadMemory(xml.data(), xml.size(), NULL, NULL, options);
  if (doc) {
    m_doc =
      Resource(NEWOBJ(XmlDocWrapper)(doc, c_SimpleXMLElement::s_class_name));
    m_node = xmlDocGetRootElement(doc);
    if (m_node) {
      m_children = create_children(m_doc, m_node, ns, is_prefix);
      m_attributes = collect_attributes(m_node, ns, is_prefix);
    }
  } else {
    throw Object(SystemLib::AllocExceptionObject(
        "String could not be parsed as XML"));
  }
}

Variant c_SimpleXMLElement::t_xpath(CStrRef path) {
  if (m_is_attribute || !m_node) {
    return uninit_null();
  }

  xmlDocPtr doc = m_node->doc;

  int nsnbr = 0;
  xmlNsPtr *ns = xmlGetNsList(doc, m_node);
  if (ns != NULL) {
    while (ns[nsnbr] != NULL) {
      nsnbr++;
    }
  }

  if (m_xpath == NULL) {
    m_xpath = xmlXPathNewContext(doc);
  }
  m_xpath->node = m_node;
  m_xpath->namespaces = ns;
  m_xpath->nsNr = nsnbr;

  xmlXPathObjectPtr retval = xmlXPathEval((xmlChar *)path.data(), m_xpath);
  if (ns != NULL) {
    xmlFree(ns);
    m_xpath->namespaces = NULL;
    m_xpath->nsNr = 0;
  }

  if (!retval) {
    return false;
  }

  xmlNodeSetPtr result = retval->nodesetval;
  if (!result) {
    xmlXPathFreeObject(retval);
    return false;
  }

  Array ret = Array::Create();
  for (int i = 0; i < result->nodeNr; ++i) {
    xmlNodePtr nodeptr = result->nodeTab[i];
    Object sub;
    /**
     * Detect the case where the last selector is text(), simplexml
     * always accesses the text() child by default, therefore we assign
     * to the parent node.
     */
    switch (nodeptr->type) {
    case XML_TEXT_NODE:
      sub = create_element(m_doc, nodeptr->parent, String(), false);
      break;
    case XML_ELEMENT_NODE:
      sub = create_element(m_doc, nodeptr, String(), false);
      break;
    case XML_ATTRIBUTE_NODE:
      sub = create_element(m_doc, nodeptr->parent, String(), false);
      break;
    default:
      break;
    }
    ret.append(sub);
  }

  xmlXPathFreeObject(retval);
  return ret;
}

bool c_SimpleXMLElement::t_registerxpathnamespace(CStrRef prefix, CStrRef ns) {
  if (m_node) {
    if (!m_xpath) {
      m_xpath = xmlXPathNewContext(m_node->doc);
    }
    return xmlXPathRegisterNs(m_xpath, (xmlChar *)prefix.data(),
                              (xmlChar *)ns.data()) == 0;
  }
  return false;
}

Variant c_SimpleXMLElement::t_asxml(CStrRef filename /* = "" */) {
  if (!m_node) return false;

  if (!filename.empty()) {
    std::string translated = File::TranslatePath(filename).data();

    if (m_node->parent && m_node->parent->type == XML_DOCUMENT_NODE) {
      int bytes = xmlSaveFile(translated.c_str(), (xmlDocPtr)m_node->doc);
      return bytes != -1;
    }

    xmlOutputBufferPtr outbuf =
      xmlOutputBufferCreateFilename(translated.c_str(), NULL, 0);
    if (outbuf == NULL) {
      return false;
    }
    xmlNodeDumpOutput(outbuf, m_node->doc, m_node, 0, 0,
                      (char*)m_node->doc->encoding);
    xmlOutputBufferClose(outbuf);
    return true;
  }

  xmlChar *strval;
  int strval_len;
  if (m_node->parent && m_node->parent->type == XML_DOCUMENT_NODE) {
    xmlDocDumpMemory(m_node->doc, &strval, &strval_len);
    String ret((char *)strval, strval_len, CopyString);
    xmlFree(strval);
    return ret;
  }

  xmlOutputBufferPtr outbuf = xmlAllocOutputBuffer(NULL);
  if (outbuf == NULL) {
    return false;
  }
  xmlNodeDumpOutput(outbuf, m_node->doc, m_node, 0, 0,
                    (char*)m_node->doc->encoding);
  xmlOutputBufferFlush(outbuf);
  String ret((char *)xmlOutputBufferGetContent(outbuf),
                     xmlOutputBufferGetSize(outbuf), CopyString);
  xmlOutputBufferClose(outbuf);
  return ret;
}

Array c_SimpleXMLElement::t_getnamespaces(bool recursive /* = false */) {
  Array ret = Array::Create();
  if (m_node) {
    if (m_node->type == XML_ELEMENT_NODE) {
      add_namespaces(ret, m_node, recursive);
    } else if (m_node->type == XML_ATTRIBUTE_NODE && m_node->ns) {
      add_namespace_name(ret, m_node->ns);
    }
  }
  return ret;
}

Array c_SimpleXMLElement::t_getdocnamespaces(bool recursive /* = false */) {
  Array ret = Array::Create();
  if (m_node) {
    add_registered_namespaces(ret, xmlDocGetRootElement(m_node->doc),
                              recursive);
  }
  return ret;
}

Object c_SimpleXMLElement::t_children(CStrRef ns /* = "" */,
                                      bool is_prefix /* = false */) {
  if (m_is_attribute) {
    return Object();
  }

  Object obj = create_object(m_doc.getTyped<XmlDocWrapper>()->
                             getClass(), Array(), false);
  c_SimpleXMLElement *elem = obj.getTyped<c_SimpleXMLElement>();
  elem->m_doc = m_doc;
  elem->m_node = m_node;
  elem->m_is_text = m_is_text;
  elem->m_free_text = m_free_text;
  elem->m_is_children = true;
  if (m_is_text) {
    // text nodes don't have children
  } else if (ns.empty()) {
    elem->m_children.assignRef(m_children);
  } else {
    Array props = Array::Create();
    for (ArrayIter iter(m_children.toArray()); iter; ++iter) {
      if (iter.second().isObject()) {
        c_SimpleXMLElement *elem = iter.second().toObject().
          getTyped<c_SimpleXMLElement>();
        if (elem->m_node && match_ns(elem->m_node, ns, is_prefix)) {
          props.set(iter.first(), iter.second());
        }
      } else {
        Array subnodes;
        for (ArrayIter iter2(iter.second().toArray()); iter2; ++iter2) {
          c_SimpleXMLElement *elem = iter2.second().toObject().
            getTyped<c_SimpleXMLElement>();
          if (elem->m_node && match_ns(elem->m_node, ns, is_prefix)) {
            subnodes.append(iter2.second());
          }
        }
        if (!subnodes.empty()) {
          if (subnodes.size() == 1) {
            props.set(iter.first(), subnodes[0]);
          } else {
            props.set(iter.first(), subnodes);
          }
        }
      }
    }
    elem->m_children = props;
  }
  return obj;
}

String c_SimpleXMLElement::t_getname() {
  if (m_is_children) {
    Variant first;
    ArrayIter iter(m_children.toArray());
    if (iter) {
      return iter.first();
    }
  } else if (m_node) {
    int namelen = xmlStrlen(m_node->name);
    return String((char*)m_node->name, namelen, CopyString);
  }
  return String();
}

static const StaticString s_attributes("@attributes");

Object c_SimpleXMLElement::t_attributes(CStrRef ns /* = "" */,
                                        bool is_prefix /* = false */) {
  if (m_is_attribute) {
    return Object();
  }

  Object obj = create_object(m_doc.getTyped<XmlDocWrapper>()->
                             getClass(), Array(), false);
  c_SimpleXMLElement *elem = obj.getTyped<c_SimpleXMLElement>();
  elem->m_doc = m_doc;
  elem->m_node = m_node;
  elem->m_is_attribute = true;
  if (!m_attributes.toArray().empty()) {
    if (!ns.empty()) {
      elem->m_attributes = collect_attributes(m_node, ns, is_prefix);
    } else {
      elem->m_attributes.assignRef(m_attributes);
    }
    elem->m_children.set(s_attributes, elem->m_attributes);
  }
  return obj;
}

Variant c_SimpleXMLElement::t_addchild(CStrRef qname,
                                       CStrRef value /* = null_string */,
                                       CStrRef ns /* = null_string */) {
  if (qname.empty()) {
    raise_warning("Element name is required");
    return uninit_null();
  }
  if (m_is_attribute) {
    raise_warning("Cannot add element to attributes");
    return uninit_null();
  }
  if (!m_node) {
    raise_warning("Parent is not a permanent member of the XML tree");
    return uninit_null();
  }

  xmlChar *prefix = NULL;
  xmlChar *localname = xmlSplitQName2((xmlChar *)qname.data(), &prefix);
  if (localname == NULL) {
    localname = xmlStrdup((xmlChar *)qname.data());
  }

  xmlNsPtr nsptr = NULL;
  xmlNodePtr newnode = xmlNewChild(m_node, NULL, localname,
                                   (xmlChar *)value.data());
  if (!ns.isNull()) {
    if (ns.empty()) {
      newnode->ns = NULL;
      nsptr = xmlNewNs(newnode, (xmlChar *)ns.data(), prefix);
    } else {
      nsptr = xmlSearchNsByHref(m_node->doc, m_node, (xmlChar *)ns.data());
      if (nsptr == NULL) {
        nsptr = xmlNewNs(newnode, (xmlChar *)ns.data(), prefix);
      }
      newnode->ns = nsptr;
    }
  }

  String newname((char*)localname, CopyString);
  String newns((char*)prefix, CopyString);
  xmlFree(localname);
  if (prefix) {
    xmlFree(prefix);
  }

  Object child = create_element(m_doc, newnode, newns, false);
  if (m_children.toArray().exists(newname)) {
    Variant &tmp = m_children.lvalAt(newname);
    if (tmp.isArray()) {
      tmp.append(child);
    } else {
      Array arr;
      arr.append(tmp);
      arr.append(child);
      m_children.set(newname, arr);
    }
  } else {
    m_children.set(newname, child);
  }
  return child;
}

void c_SimpleXMLElement::t_addattribute(CStrRef qname,
                                        CStrRef value /* = null_string */,
                                        CStrRef ns /* = null_string */) {
  if (qname.empty()) {
    raise_warning("Attribute name is required");
    return;
  }

  if (m_node && m_node->type != XML_ELEMENT_NODE) {
    m_node = m_node->parent;
  }
  if (!m_node) {
    raise_warning("Unable to locate parent Element");
    return;
  }

  xmlChar *prefix = NULL;
  xmlChar *localname = xmlSplitQName2((xmlChar *)qname.data(), &prefix);
  if (localname == NULL) {
    localname = xmlStrdup((xmlChar *)qname.data());
  }

  xmlAttrPtr attrp = xmlHasNsProp(m_node, localname, (xmlChar *)ns.data());
  if (attrp && attrp->type != XML_ATTRIBUTE_DECL) {
    xmlFree(localname);
    if (prefix != NULL) {
      xmlFree(prefix);
    }
    raise_warning("Attribute already exists");
    return;
  }

  xmlNsPtr nsptr = NULL;
  if (!ns.isNull()) {
    nsptr = xmlSearchNsByHref(m_node->doc, m_node, (xmlChar *)ns.data());
    if (nsptr == NULL) {
      nsptr = xmlNewNs(m_node, (xmlChar *)ns.data(), prefix);
    }
  }

  attrp = xmlNewNsProp(m_node, nsptr, localname, (xmlChar *)value.data());
  m_attributes.set(String((char*)localname, CopyString), value);

  xmlFree(localname);
  if (prefix != NULL) {
    xmlFree(prefix);
  }
}

String c_SimpleXMLElement::t___tostring() {
  Variant prop;
  ArrayIter iter(m_children.toArray());
  if (iter) {
    prop = iter.second();
    if (prop.isString()) {
      return prop.toString();
    }
    if (prop.isObject()) {
      c_SimpleXMLElement *elem =
        prop.toObject().getTyped<c_SimpleXMLElement>();
      if (elem->m_is_text && elem->m_free_text) {
        return prop.toString();
      }
    }
  }
  return "";
}

Variant c_SimpleXMLElement::t___get(Variant name) {
  Variant ret = m_children[name];
  if (ret.isArray()) {
    ret = ret[0];
  }
  if (ret.isObject()) {
    c_SimpleXMLElement *elem = ret.toObject().getTyped<c_SimpleXMLElement>();
    Object obj = create_object(m_doc.getTyped<XmlDocWrapper>()->
                               getClass(), Array(), false);
    c_SimpleXMLElement *e = obj.getTyped<c_SimpleXMLElement>();
    e->m_doc = elem->m_doc;
    e->m_node = elem->m_node;
    e->m_children.assignRef(elem->m_children);
    e->m_attributes.assignRef(elem->m_attributes);
    e->m_is_text = elem->m_is_text;
    e->m_is_property = true;
    return obj;
  }
  if (ret.isNull()) {
    return create_object(o_getClassName(), Array(), false);
  }
  return ret;
}

Variant c_SimpleXMLElement::t___unset(Variant name) {
  if (m_node == NULL) return uninit_null();

  Variant node;
  if (m_is_attribute) {
    node = m_attributes[name];
  } else {
    node = m_children[name];
  }

  if (node.isObject()) {
    c_SimpleXMLElement *elem =
      node.toObject().getTyped<c_SimpleXMLElement>();
    if (elem->m_node) {
      xmlUnlinkNode(elem->m_node);
    }
  } else if (node.isArray()) {
    for (ArrayIter iter(node.toArray()); iter; ++iter) {
      c_SimpleXMLElement *elem = iter.second().toObject().
        getTyped<c_SimpleXMLElement>();
      if (elem->m_node) {
        xmlUnlinkNode(elem->m_node);
      }
    }
  }

  if (m_is_attribute) {
    m_attributes.remove(name);
  } else {
    m_children.remove(name);
  }
  return uninit_null();
}

bool c_SimpleXMLElement::t___isset(Variant name) {
  if (m_node) {
    if (m_is_attribute) {
      return m_attributes.toArray().exists(name);
    } else {
      return m_children.toArray().exists(name);
    }
  }
  return false;
}

static void change_node_zval(xmlNodePtr node, CStrRef value) {
  if (value.empty()) {
    xmlNodeSetContentLen(node, (xmlChar *)"", 0);
  } else {
    xmlChar *buffer =
      xmlEncodeEntitiesReentrant(node->doc, (xmlChar *)value.data());
    int buffer_len = xmlStrlen(buffer);
    if (buffer) {
      xmlNodeSetContentLen(node, buffer, buffer_len);
      xmlFree(buffer);
    }
  }
}

Variant c_SimpleXMLElement::t___set(Variant name, Variant value) {
  if (m_node == NULL) return uninit_null();

  String svalue = value.toString();
  xmlChar *sv = svalue.empty() ? NULL : (xmlChar *)svalue.data();
  String sname = name.toString();

  Variant node;
  if (m_is_attribute) {
    node = m_attributes[name];
  } else {
    node = m_children[name];
  }

  xmlNodePtr newnode = NULL;
  if (node.isObject()) {
    c_SimpleXMLElement *elem =
      node.toObject().getTyped<c_SimpleXMLElement>();
    if (elem->m_node) {
      xmlNodePtr tempnode;
      while ((tempnode = (xmlNodePtr)elem->m_node->children)) {
        xmlUnlinkNode(tempnode);
      }
      elem->m_children = Array::Create();
      change_node_zval(elem->m_node, svalue);
      newnode = elem->m_node;
    }
  } else if (node.isArray()) {
    raise_warning("Cannot assign to an array of nodes "
                  "(duplicate subnodes or attr detected)");
  } else if (m_is_attribute) {
    if (name.isInteger()) {
      raise_warning("Cannot change attribute number %" PRId64
                    " when only %zd attributes exist", name.toInt64(),
                    m_attributes.toArray().size());
    } else {
      newnode = (xmlNodePtr)xmlNewProp(m_node, (xmlChar *)sname.data(), sv);
    }
  } else {
    if (sname.empty() || name.isInteger()) {
      newnode = xmlNewTextChild(m_node->parent, m_node->ns,
                                m_node->name, sv);
    } else {
      newnode = xmlNewTextChild(m_node, m_node->ns,
                                (xmlChar *)sname.data(), sv);
    }
  }

  if (newnode) {
    String ns((char*)m_node->ns, CopyString);
    Object child = create_element(m_doc, newnode, ns, false);
    if (m_is_attribute) {
      m_attributes.set(name, child);
      m_children.set(s_attributes, m_attributes);
    } else {
      m_children.set(name, child);
    }
  }

  return uninit_null();
}

bool c_SimpleXMLElement::o_toBooleanImpl() const noexcept {
  return (m_node || getDynProps().size()) &&
         (m_node->parent->type != XML_DOCUMENT_NODE ||
          m_children.toArray().size());
}

int64_t c_SimpleXMLElement::o_toInt64Impl() const noexcept {
  Variant prop;
  ArrayIter iter(m_children.toArray());
  if (iter) {
    prop = iter.second();
  }
  return prop.toString().toInt64();
}

double c_SimpleXMLElement::o_toDoubleImpl() const noexcept {
  Variant prop;
  ArrayIter iter(m_children.toArray());
  if (iter) {
    prop = iter.second();
  }
  return prop.toString().toDouble();
}

Array c_SimpleXMLElement::o_toArray() const {
  if (m_attributes.toArray().empty()) {
    return m_children.toArray();
  }
  Array ret;
  ret.set(s_attributes, m_attributes);
  ret += m_children;
  return ret;
}

Variant c_SimpleXMLElement::t_getiterator() {
  c_SimpleXMLElementIterator *iter = NEWOBJ(c_SimpleXMLElementIterator)();
  iter->set_parent(this);
  return Object(iter);
}

int64_t c_SimpleXMLElement::t_count() {
  if (m_is_attribute) {
    return m_attributes.toArray().size();
  }
  if (m_is_property) {
    int64_t n = 0; Variant var(this);
    for (ArrayIter iter = var.begin(); !iter.end(); iter.next()) {
      ++n;
    }
    return n;
  }
  return m_children.toArray().size();
}

///////////////////////////////////////////////////////////////////////////////
// implementing ArrayAccess

bool c_SimpleXMLElement::t_offsetexists(CVarRef index) {
  if (index.isInteger()) {
    int64_t n = 0; int64_t nIndex = index.toInt64(); Variant var(this);
    for (ArrayIter iter = var.begin(); !iter.end(); iter.next()) {
      if (n++ == nIndex) {
        return true;
      }
    }
    return false;
  }
  return m_attributes.toArray().exists(index);
}

Variant c_SimpleXMLElement::t_offsetget(CVarRef index) {
  if (index.isInteger()) {
    if (m_is_property) {
      int64_t n = 0; int64_t nIndex = index.toInt64(); Variant var(this);
      for (ArrayIter iter = var.begin(); !iter.end(); iter.next()) {
        if (n++ == nIndex) {
          return iter.second();
        }
      }
      return this;
    }
    return m_children[index];
  }
  return m_attributes[index];
}

void c_SimpleXMLElement::t_offsetset(CVarRef index, CVarRef newvalue) {
  if (index.isInteger()) {
    raise_error("unable to replace a SimpleXMLElement node");
    return;
  }
  String name = index.toString();
  if (name.empty()) {
    raise_error("cannot create unnamed attribute");
    return;
  }

  String sv = newvalue.toString();
  if (m_attributes.toArray().exists(name)) {
    t_offsetunset(index);
  }

  if (m_node == NULL || m_is_text) {
    raise_error("cannot create attribute on this node");
    return;
  }
  xmlNodePtr newnode = (xmlNodePtr)xmlNewProp(m_node, (xmlChar *)name.data(),
                                              (xmlChar*)sv.data());
  if (newnode) {
    m_attributes.set(name, sv);
  }
}

void c_SimpleXMLElement::t_offsetunset(CVarRef index) {
  if (index.isInteger()) {
    raise_error("unable to remove a SimpleXMLElement node");
    return;
  }

  String name = index.toString();
  if (name.empty()) {
    raise_error("cannot remove unnamed attribute");
    return;
  }

  if (m_attributes.toArray().exists(name) && m_node) {
    for (xmlAttrPtr attr = m_node->properties; attr; attr = attr->next) {
      if (String((char*)attr->name, xmlStrlen(attr->name), CopyString) ==
          name) {
        xmlUnlinkNode((xmlNodePtr)attr);
        break;
      }
    }
  }

  m_attributes.remove(name);
}

///////////////////////////////////////////////////////////////////////////////

c_SimpleXMLElementIterator::c_SimpleXMLElementIterator(Class* cb) :
    ExtObjectData(cb), m_parent(), m_iter1(NULL), m_iter2(NULL) {
}

c_SimpleXMLElementIterator::~c_SimpleXMLElementIterator() {
  c_SimpleXMLElementIterator::sweep();
}

void c_SimpleXMLElementIterator::sweep() {
  delete m_iter1;
  delete m_iter2;
}

void c_SimpleXMLElementIterator::set_parent(c_SimpleXMLElement* parent) {
  m_parent = parent;
  reset_iterator();
}

void c_SimpleXMLElementIterator::reset_iterator() {
  assert(m_parent.get() != NULL);
  delete m_iter1; m_iter1 = NULL;
  delete m_iter2; m_iter2 = NULL;

  if (m_parent->m_is_attribute) {
    m_iter1 = new ArrayIter(m_parent->m_attributes.toArray());
    return;
  }

  // When I'm a node like $node->name, we iterate through all my siblings with
  // same name of mine.
  if (m_parent->m_is_property) {
    String name = m_parent->t_getname();
    Object obj = create_element(m_parent->m_doc, m_parent->m_node->parent,
                                "", false);
    m_parent = obj.getTyped<c_SimpleXMLElement>();
    Variant children = m_parent->m_children[name];
    m_parent->m_children = CREATE_MAP1(name, children);
    // fall through
  }

  if (m_parent->m_is_text) {
    return;
  }

  if (m_parent->m_children.toArray().size() == 1) {
    ArrayIter iter(m_parent->m_children.toArray());
    if (iter.second().isObject()) {
      c_SimpleXMLElement *elem = iter.second().toObject().
        getTyped<c_SimpleXMLElement>();
      if (elem->m_is_text && elem->m_free_text) {
        return;
      }
    }
  }

  m_iter1 = new ArrayIter(m_parent->m_children.toArray());
  if (!m_iter1->end() && m_iter1->second().isArray()) {
    m_iter2 = new ArrayIter(m_iter1->second().toArray());
  }
}

void c_SimpleXMLElementIterator::t___construct() {
}

Variant c_SimpleXMLElementIterator::t_current() {
  if (m_iter1 == NULL) return uninit_null();
  if (m_parent->m_is_attribute) {
    return m_iter1->second();
  }

  ArrayIter *iter = m_iter2;
  if (iter == NULL && m_iter1->second().isObject()) {
    iter = m_iter1;
  }

  if (iter) {
    return iter->second();
  }

  assert(false);
  return uninit_null();
}

Variant c_SimpleXMLElementIterator::t_key() {
  if (m_iter1) {
    return m_iter1->first();
  }
  return uninit_null();
}

Variant c_SimpleXMLElementIterator::t_next() {
  if (m_iter1 == NULL) return uninit_null();
  if (m_parent->m_is_attribute) {
    m_iter1->next();
    return uninit_null();
  }

  if (m_iter2) {
    m_iter2->next();
    if (!m_iter2->end()) {
      return uninit_null();
    }
    delete m_iter2; m_iter2 = NULL;
  }
  m_iter1->next();
  while (!m_iter1->end()) {
    if (m_iter1->second().isArray()) {
      m_iter2 = new ArrayIter(m_iter1->second().toArray());
      break;
    }
    if (m_iter1->second().isObject()) {
      break;
    }
    m_iter1->next();
  }
  return uninit_null();
}

Variant c_SimpleXMLElementIterator::t_rewind() {
  reset_iterator();
  return uninit_null();
}

Variant c_SimpleXMLElementIterator::t_valid() {
  return m_iter1 && !m_iter1->end();
}

///////////////////////////////////////////////////////////////////////////////
// LibXMLError

c_LibXMLError::c_LibXMLError(Class* cb) :
    ExtObjectData(cb) {
}
c_LibXMLError::~c_LibXMLError() {
}
void c_LibXMLError::t___construct() {
}

///////////////////////////////////////////////////////////////////////////////
// libxml

class xmlErrorVec : public std::vector<xmlError> {
public:
  ~xmlErrorVec() {
    reset();
  }

  void reset() {
    for (unsigned int i = 0; i < size(); i++) {
      xmlResetError(&at(i));
    }
    clear();
  }
};

class LibXmlErrors : public RequestEventHandler {
public:
  virtual void requestInit() {
    m_use_error = false;
    m_errors.reset();
    xmlParserInputBufferCreateFilenameDefault(NULL);
  }
  virtual void requestShutdown() {
    m_use_error = false;
    m_errors.reset();
  }

  bool m_use_error;
  xmlErrorVec m_errors;
};

IMPLEMENT_STATIC_REQUEST_LOCAL(LibXmlErrors, s_libxml_errors);
bool libxml_use_internal_error() {
  return s_libxml_errors->m_use_error;
}
extern void libxml_add_error(const std::string &msg) {
  xmlErrorVec *error_list = &s_libxml_errors->m_errors;

  error_list->resize(error_list->size() + 1);
  xmlError &error_copy = error_list->back();
  memset(&error_copy, 0, sizeof(xmlError));

  error_copy.domain = 0;
  error_copy.code = XML_ERR_INTERNAL_ERROR;
  error_copy.level = XML_ERR_ERROR;
  error_copy.line = 0;
  error_copy.node = NULL;
  error_copy.int1 = 0;
  error_copy.int2 = 0;
  error_copy.ctxt = NULL;
  error_copy.message = (char*)xmlStrdup((const xmlChar*)msg.c_str());
  error_copy.file = NULL;
  error_copy.str1 = NULL;
  error_copy.str2 = NULL;
  error_copy.str3 = NULL;
}

static void libxml_error_handler(void *userData, xmlErrorPtr error) {
  xmlErrorVec *error_list = &s_libxml_errors->m_errors;

  error_list->resize(error_list->size() + 1);
  xmlError &error_copy = error_list->back();
  memset(&error_copy, 0, sizeof(xmlError));

  if (error) {
    xmlCopyError(error, &error_copy);
  } else {
    error_copy.code = XML_ERR_INTERNAL_ERROR;
    error_copy.level = XML_ERR_ERROR;
  }
}

static const StaticString s_level("level");
static const StaticString s_code("code");
static const StaticString s_column("column");
static const StaticString s_message("message");
static const StaticString s_file("file");
static const StaticString s_line("line");

static Object create_libxmlerror(xmlError &error) {
  Object ret(NEWOBJ(c_LibXMLError)());
  ret->o_set(s_level,   error.level);
  ret->o_set(s_code,    error.code);
  ret->o_set(s_column,  error.int2);
  ret->o_set(s_message, String(error.message, CopyString));
  ret->o_set(s_file,    String(error.file, CopyString));
  ret->o_set(s_line,    error.line);
  return ret;
}

Variant f_libxml_get_errors() {
  xmlErrorVec *error_list = &s_libxml_errors->m_errors;
  Array ret = Array::Create();
  for (unsigned int i = 0; i < error_list->size(); i++) {
    ret.append(create_libxmlerror(error_list->at(i)));
  }
  return ret;
}

Variant f_libxml_get_last_error() {
  xmlErrorPtr error = xmlGetLastError();
  if (error) {
    return create_libxmlerror(*error);
  }
  return false;
}

void f_libxml_clear_errors() {
  xmlResetLastError();
  s_libxml_errors->m_errors.reset();
}

bool f_libxml_use_internal_errors(CVarRef use_errors /* = null_variant */) {
  bool ret = (xmlStructuredError == libxml_error_handler);
  if (!use_errors.isNull()) {
    if (!use_errors.toBoolean()) {
      xmlSetStructuredErrorFunc(NULL, NULL);
      s_libxml_errors->m_use_error = false;
      s_libxml_errors->m_errors.reset();
    } else {
      xmlSetStructuredErrorFunc(NULL, libxml_error_handler);
      s_libxml_errors->m_use_error = true;
    }
  }
  return ret;
}

void f_libxml_set_streams_context(CResRef streams_context) {
  throw NotImplementedException(__func__);
}

static xmlParserInputBufferPtr
hphp_libxml_input_buffer_noload(const char *URI, xmlCharEncoding enc) {
  return NULL;
}

bool f_libxml_disable_entity_loader(bool disable /* = true */) {
  xmlParserInputBufferCreateFilenameFunc old;

  if (disable) {
    old = xmlParserInputBufferCreateFilenameDefault(hphp_libxml_input_buffer_noload);
  } else {
    old = xmlParserInputBufferCreateFilenameDefault(NULL);
  }
  return (old == hphp_libxml_input_buffer_noload);
}

///////////////////////////////////////////////////////////////////////////////
}
