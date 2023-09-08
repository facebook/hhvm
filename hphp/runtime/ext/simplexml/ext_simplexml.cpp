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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/ext/simplexml/ext_simplexml.h"
#include <vector>
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/simplexml/ext_simplexml_include.h"
#include "hphp/runtime/ext/domdocument/ext_domdocument.h"
#include "hphp/runtime/ext/std/ext_std_file.h"
#include "hphp/runtime/ext/libxml/ext_libxml.h"
#include "hphp/system/systemlib.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/runtime/vm/native-prop-handler.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// Iter type

enum SXE_ITER {
  SXE_ITER_NONE     = 0,
  SXE_ITER_ELEMENT  = 1,
  SXE_ITER_CHILD    = 2,
  SXE_ITER_ATTRLIST = 3
};

///////////////////////////////////////////////////////////////////////////////
// NativeData definitions

struct SimpleXMLElement {
  SimpleXMLElement() {
    assertx(Native::object<SimpleXMLElement>(this)->getVMClass()->
           rtAttribute(Class::CallToImpl));
  }
  SimpleXMLElement& operator=(const SimpleXMLElement &src) {
    iter.isprefix = src.iter.isprefix;
    if (src.iter.name != nullptr) {
      iter.name = xmlStrdup((xmlChar*)src.iter.name);
    }
    if (src.iter.nsprefix != nullptr) {
      iter.nsprefix = xmlStrdup((xmlChar*)src.iter.nsprefix);
    }
    iter.type = src.iter.type;

    if (src.nodep()) {
      node = libxml_register_node(
        xmlDocCopyNode(src.nodep(), src.docp(), 1)
      );
    }

    return *this;
  }
  ~SimpleXMLElement() { sweep(); }
  void sweep() {
    if (iter.name) {
      xmlFree(iter.name);
    }
    if (iter.nsprefix) {
      xmlFree(iter.nsprefix);
    }
    if (xpath) {
      xmlXPathFreeContext(xpath);
    }
  }

  xmlNodePtr nodep() const {
    return node ? node->nodep() : nullptr;
  }

  xmlDocPtr docp() const {
    return node ? node->docp() : nullptr;
  }

  XMLNode node{nullptr};
  xmlXPathContextPtr xpath{nullptr};
  struct {
    xmlChar* name{nullptr};
    xmlChar* nsprefix{nullptr};
    bool     isprefix{false};
    SXE_ITER type{SXE_ITER_NONE};
    Object   data;
  } iter;
};

struct SimpleXMLElementIterator {
  SimpleXMLElement* sxe() {
    assertx(m_sxe->instanceof(SimpleXMLElementLoader::classof()));
    return Native::data<SimpleXMLElement>(m_sxe.get());
  }

  void setSxe(const Object& sxe) {
    assertx(sxe->instanceof(SimpleXMLElementLoader::classof()));
    m_sxe = Object(sxe.get());
  }

 private:
  Object m_sxe;
};

using SimpleXMLIterator = SimpleXMLElement;

///////////////////////////////////////////////////////////////////////////////
// Helpers

#define SKIP_TEXT(__p) \
  if ((__p)->type == XML_TEXT_NODE) { \
    goto next_iter; \
  }

#define SXE_NS_PREFIX(ns) (ns->prefix ? (char*)ns->prefix : "")

static inline void sxe_add_namespace_name(Array& ret, xmlNsPtr ns) {
  String prefix = String(SXE_NS_PREFIX(ns));
  if (!ret.exists(prefix)) {
    ret.set(prefix, String((char*)ns->href, CopyString));
  }
}

static void sxe_add_registered_namespaces(SimpleXMLElement* sxe,
                                          xmlNodePtr node, bool recursive,
                                          Array& return_value) {
  if (node != nullptr && node->type == XML_ELEMENT_NODE) {
    xmlNsPtr ns = node->nsDef;
    while (ns != nullptr) {
      sxe_add_namespace_name(return_value, ns);
      ns = ns->next;
    }
    if (recursive) {
      node = node->children;
      while (node) {
        sxe_add_registered_namespaces(sxe, node, recursive, return_value);
        node = node->next;
      }
    }
  }
}

static void sxe_add_namespaces(SimpleXMLElement* sxe, xmlNodePtr node,
                               bool recursive, Array& return_value) {
  if (node->ns) {
    sxe_add_namespace_name(return_value, node->ns);
  }

  xmlAttrPtr attr = node->properties;
  while (attr) {
    if (attr->ns) {
      sxe_add_namespace_name(return_value, attr->ns);
    }
    attr = attr->next;
  }

  if (recursive) {
    node = node->children;
    while (node) {
      if (node->type == XML_ELEMENT_NODE) {
        sxe_add_namespaces(sxe, node, recursive, return_value);
      }
      node = node->next;
    }
  }
}

static Object _node_as_zval(SimpleXMLElement* sxe, xmlNodePtr node,
                            SXE_ITER itertype, const char* name,
                            const xmlChar* nsprefix, bool isprefix) {
  auto sxeObj = Native::object<SimpleXMLElement>(sxe);
  Object obj = create_object(sxeObj->getClassName(), Array(), false);
  auto subnode = Native::data<SimpleXMLElement>(obj.get());
  subnode->iter.type = itertype;
  if (name) {
    subnode->iter.name = xmlStrdup((xmlChar*)name);
  }
  if (nsprefix && *nsprefix) {
    subnode->iter.nsprefix = xmlStrdup(nsprefix);
    subnode->iter.isprefix = isprefix;
  }
  subnode->node = libxml_register_node(node);
  return obj;
}

static inline bool match_ns(SimpleXMLElement* /*sxe*/, xmlNodePtr node,
                            xmlChar* name, bool prefix) {
  if (name == nullptr && (node->ns == nullptr || node->ns->prefix == nullptr)) {
    return true;
  }

  if (RuntimeOption::SimpleXMLEmptyNamespaceMatchesAll &&
      (name == nullptr || *name == '\0')) {
    return true;
  }

  if (node->ns &&
      !xmlStrcmp(prefix ? node->ns->prefix : node->ns->href, name)) {
    return true;
  }

  return false;
}

static xmlNodePtr sxe_get_element_by_offset(SimpleXMLElement* sxe,
                                            long offset, xmlNodePtr node,
                                            long* cnt) {
  if (sxe->iter.type == SXE_ITER_NONE) {
    if (offset == 0) {
      if (cnt) {
        *cnt = 0;
      }
      return node;
    } else {
      return nullptr;
    }
  }

  long nodendx = 0;
  while (node && nodendx <= offset) {
    SKIP_TEXT(node)
    if (node->type == XML_ELEMENT_NODE &&
        match_ns(sxe, node, sxe->iter.nsprefix, sxe->iter.isprefix)) {
      if (sxe->iter.type == SXE_ITER_CHILD ||
          (sxe->iter.type == SXE_ITER_ELEMENT
           && !xmlStrcmp(node->name, sxe->iter.name))) {
        if (nodendx == offset) {
          break;
        }
        nodendx++;
      }
    }
next_iter:
    node = node->next;
  }

  if (cnt) {
    *cnt = nodendx;
  }

  return node;
}

static xmlNodePtr php_sxe_iterator_fetch(SimpleXMLElement* sxe,
                                         xmlNodePtr node, int use_data) {
  xmlChar* prefix  = sxe->iter.nsprefix;
  bool isprefix  = sxe->iter.isprefix;
  bool test_elem = sxe->iter.type == SXE_ITER_ELEMENT  && sxe->iter.name;
  bool test_attr = sxe->iter.type == SXE_ITER_ATTRLIST && sxe->iter.name;

  while (node) {
    SKIP_TEXT(node)
    if (sxe->iter.type != SXE_ITER_ATTRLIST && node->type == XML_ELEMENT_NODE) {
      if ((!test_elem || !xmlStrcmp(node->name, sxe->iter.name))
          && match_ns(sxe, node, prefix, isprefix)) {
        break;
      }
    } else if (node->type == XML_ATTRIBUTE_NODE) {
      if ((!test_attr || !xmlStrcmp(node->name, sxe->iter.name)) &&
          match_ns(sxe, node, prefix, isprefix)) {
        break;
      }
    }
next_iter:
    node = node->next;
  }

  if (node && use_data) {
    sxe->iter.data = _node_as_zval(sxe, node, SXE_ITER_NONE, nullptr, prefix,
                                   isprefix);
  }

  return node;
}

static void php_sxe_move_forward_iterator(SimpleXMLElement* sxe) {
  xmlNodePtr node = nullptr;
  auto data = sxe->iter.data;
  if (!data.isNull()) {
    assertx(data->instanceof(SimpleXMLElementLoader::classof()));
    auto intern = Native::data<SimpleXMLElement>(data.get());
    node = intern->nodep();
    sxe->iter.data.reset();
  }

  if (node) {
    php_sxe_iterator_fetch(sxe, node->next, 1);
  }
}

static xmlNodePtr php_sxe_reset_iterator(SimpleXMLElement* sxe,
                                         bool use_data) {
  if (!sxe->iter.data.isNull()) {
    sxe->iter.data.reset();
  }

  xmlNodePtr node = sxe->nodep();
  if (node) {
    switch (sxe->iter.type) {
      case SXE_ITER_ELEMENT:
      case SXE_ITER_CHILD:
      case SXE_ITER_NONE:
        node = node->children;
        break;
      case SXE_ITER_ATTRLIST:
        node = (xmlNodePtr)node->properties;
    }
    return php_sxe_iterator_fetch(sxe, node, use_data);
  }
  return nullptr;
}

static int64_t php_sxe_count_elements_helper(SimpleXMLElement* sxe) {
  Object data = sxe->iter.data;
  sxe->iter.data.reset();

  xmlNodePtr node = php_sxe_reset_iterator(sxe, false);
  int64_t count = 0;
  while (node) {
    count++;
    node = php_sxe_iterator_fetch(sxe, node->next, 0);
  }

  sxe->iter.data = data;
  return count;
}

static xmlNodePtr php_sxe_get_first_node(SimpleXMLElement* sxe,
                                         xmlNodePtr node) {
  if (sxe && sxe->iter.type != SXE_ITER_NONE) {
    php_sxe_reset_iterator(sxe, true);
    xmlNodePtr retnode = nullptr;
    if (!sxe->iter.data.isNull()) {
      assertx(sxe->iter.data->instanceof(SimpleXMLElementLoader::classof()));
      retnode = Native::data<SimpleXMLElement>(sxe->iter.data.get())->nodep();
    }
    return retnode;
  } else {
    return node;
  }
}

xmlNodePtr SimpleXMLElement_exportNode(const Object& sxe) {
  if (!sxe->instanceof(SimpleXMLElementLoader::classof())) return nullptr;
  auto data = Native::data<SimpleXMLElement>(sxe.get());
  return php_sxe_get_first_node(data, data->nodep());
}

static Object sxe_prop_dim_read(SimpleXMLElement* sxe, const Variant& member,
                                bool elements, bool attribs) {
  xmlNodePtr node = sxe->nodep();

  String name = "";
  if (member.isNull() || member.isInteger()) {
    if (sxe->iter.type != SXE_ITER_ATTRLIST) {
      attribs = false;
      elements = true;
    } else if (member.isNull()) {
      /* This happens when the user did: $sxe[]->foo = $value */
      raise_error("Cannot create unnamed attribute");
      return Object{};
    }
  } else {
    name = member.toString();
  }

  xmlAttrPtr attr = nullptr;
  bool test = false;
  if (sxe->iter.type == SXE_ITER_ATTRLIST) {
    attribs = true;
    elements = false;
    node = php_sxe_get_first_node(sxe, node);
    attr = (xmlAttrPtr)node;
    test = sxe->iter.name != nullptr;
  } else if (sxe->iter.type != SXE_ITER_CHILD) {
    node = php_sxe_get_first_node(sxe, node);
    attr = node ? node->properties : nullptr;
    test = false;
    if (member.isNull() && node && node->parent &&
        node->parent->type == XML_DOCUMENT_NODE) {
      /* This happens when the user did: $sxe[]->foo = $value */
      raise_error("Cannot create unnamed attribute");
      return Object{};
    }
  }

  Object return_value;
  if (node) {
    if (attribs) {
      if (!member.isInteger() || sxe->iter.type == SXE_ITER_ATTRLIST) {
        if (member.isInteger()) {
          int64_t nodendx = 0;
          while (attr && nodendx <= member.toInt64()) {
            if ((!test || !xmlStrcmp(attr->name, sxe->iter.name)) &&
                match_ns(sxe, (xmlNodePtr) attr, sxe->iter.nsprefix,
                         sxe->iter.isprefix)) {
              if (nodendx == member.toInt64()) {
                return_value = _node_as_zval(sxe, (xmlNodePtr) attr,
                                             SXE_ITER_NONE, nullptr,
                                             sxe->iter.nsprefix,
                                             sxe->iter.isprefix);
                break;
              }
              nodendx++;
            }
            attr = attr->next;
          }
        } else {
          while (attr) {
            if ((!test || !xmlStrcmp(attr->name, sxe->iter.name)) &&
                !xmlStrcmp(attr->name, (xmlChar*)name.data()) &&
                match_ns(sxe, (xmlNodePtr) attr, sxe->iter.nsprefix,
                         sxe->iter.isprefix)) {
              return_value = _node_as_zval(sxe, (xmlNodePtr) attr,
                                           SXE_ITER_NONE,
                                           nullptr,
                                           sxe->iter.nsprefix,
                                           sxe->iter.isprefix);
              break;
            }
            attr = attr->next;
          }
        }
      }
    }

    if (elements) {
      if (!sxe->nodep()) {
        sxe->node = libxml_register_node(node);
      }
      if (member.isNull() || member.isInteger()) {
        long cnt = 0;
        xmlNodePtr mynode = node;

        if (sxe->iter.type == SXE_ITER_CHILD) {
          node = php_sxe_get_first_node(sxe, node);
        }
        if (sxe->iter.type == SXE_ITER_NONE) {
          if (!member.isNull() && member.toInt64() > 0) {
            raise_warning("Cannot add element %s number %" PRId64 " when "
                          "only 0 such elements exist", mynode->name,
                          member.toInt64());
          }
        } else if (!member.isNull()) {
          node = sxe_get_element_by_offset(sxe, member.toInt64(), node, &cnt);
        } else {
          node = nullptr;
        }
        if (node) {
          return_value = _node_as_zval(sxe, node, SXE_ITER_NONE, nullptr,
                                       sxe->iter.nsprefix, sxe->iter.isprefix);
        }
        // Zend would check here if this is a write operation, but HHVM always
        // handles that with offsetSet so we just want to return nullptr here.
      } else {
#if SXE_ELEMENT_BY_NAME
        int newtype;

        node = sxe->nodep();
        node = sxe_get_element_by_name(sxe, node, &name.data(), &newtype);
        if (node) {
          return_value = _node_as_zval(sxe, node, newtype, name.data(),
                                       sxe->iter.nsprefix, sxe->iter.isprefix);
        }
#else
        return_value = _node_as_zval(sxe, node, SXE_ITER_ELEMENT, name.data(),
                                     sxe->iter.nsprefix, sxe->iter.isprefix);
#endif
      }
    }
  }

  return return_value;
}

static void change_node_zval(xmlNodePtr node, const Variant& value) {
  if (value.isNull()) {
    xmlNodeSetContentLen(node, (xmlChar*)"", 0);
    return;
  }
  if (value.isInteger() || value.isBoolean() || value.isDouble() ||
      value.isNull() || value.isString()) {
      xmlChar* buffer =
        xmlEncodeEntitiesReentrant(node->doc,
                                   (xmlChar*)value.toString().data());
      int64_t buffer_len = xmlStrlen(buffer);
      /* check for nullptr buffer in case of
       * memory error in xmlEncodeEntitiesReentrant */
      if (buffer) {
        xmlNodeSetContentLen(node, buffer, buffer_len);
        xmlFree(buffer);
      }
  } else {
    raise_warning("It is not possible to assign complex types to nodes");
  }
}

static void sxe_prop_dim_delete(SimpleXMLElement* sxe, const Variant& member,
                                bool elements, bool attribs) {
  xmlNodePtr node = sxe->nodep();

  if (member.isInteger()) {
    if (sxe->iter.type != SXE_ITER_ATTRLIST) {
      attribs = false;
      elements = true;
      if (sxe->iter.type == SXE_ITER_CHILD) {
        node = php_sxe_get_first_node(sxe, node);
      }
    }
  }

  xmlAttrPtr attr = nullptr;
  bool test = 0;
  if (sxe->iter.type == SXE_ITER_ATTRLIST) {
    attribs = true;
    elements = false;
    node = php_sxe_get_first_node(sxe, node);
    attr = (xmlAttrPtr)node;
    test = sxe->iter.name != nullptr;
  } else if (sxe->iter.type != SXE_ITER_CHILD) {
    node = php_sxe_get_first_node(sxe, node);
    attr = node ? node->properties : nullptr;
    test = false;
  }

  if (node) {
    if (attribs) {
      if (member.isInteger()) {
        int64_t nodendx = 0;

        while (attr && nodendx <= member.toInt64()) {
          if ((!test || !xmlStrcmp(attr->name, sxe->iter.name)) &&
              match_ns(sxe, (xmlNodePtr) attr, sxe->iter.nsprefix,
                       sxe->iter.isprefix)) {
            if (nodendx == member.toInt64()) {
              libxml_register_node((xmlNodePtr) attr)->unlink();
              break;
            }
            nodendx++;
          }
          attr = attr->next;
        }
      } else {
        xmlAttrPtr anext;
        while (attr) {
          anext = attr->next;
          if ((!test || !xmlStrcmp(attr->name, sxe->iter.name)) &&
              !xmlStrcmp(attr->name, (xmlChar*)member.toString().data()) &&
              match_ns(sxe, (xmlNodePtr) attr, sxe->iter.nsprefix,
                       sxe->iter.isprefix)) {
            libxml_register_node((xmlNodePtr) attr)->unlink();
            break;
          }
          attr = anext;
        }
      }
    }

    if (elements) {
      if (member.isInteger()) {
        if (sxe->iter.type == SXE_ITER_CHILD) {
          node = php_sxe_get_first_node(sxe, node);
        }
        node = sxe_get_element_by_offset(sxe, member.toInt64(), node, nullptr);
        if (node) {
          libxml_register_node(node)->unlink();
        }
      } else {
        node = node->children;
        xmlNodePtr nnext;
        while (node) {
          nnext = node->next;

          SKIP_TEXT(node);

          if (!xmlStrcmp(node->name, (xmlChar*)member.toString().data())) {
            libxml_register_node(node)->unlink();
          }

next_iter:
          node = nnext;
        }
      }
    }
  }
}

static bool sxe_prop_dim_exists(SimpleXMLElement* sxe, const Variant& member,
                                bool check_empty, bool elements, bool attribs) {
  xmlNodePtr node = sxe->nodep();

  if (member.isInteger()) {
    if (sxe->iter.type != SXE_ITER_ATTRLIST) {
      attribs = false;
      elements = true;
      if (sxe->iter.type == SXE_ITER_CHILD) {
        node = php_sxe_get_first_node(sxe, node);
      }
    }
  }

  xmlAttrPtr attr = nullptr;
  bool test = false;
  if (sxe->iter.type == SXE_ITER_ATTRLIST) {
    attribs = true;
    elements = false;
    node = php_sxe_get_first_node(sxe, node);
    attr = (xmlAttrPtr)node;
    test = sxe->iter.name != nullptr;
  } else if (sxe->iter.type != SXE_ITER_CHILD) {
    node = php_sxe_get_first_node(sxe, node);
    attr = node ? node->properties : nullptr;
    test = false;
  }

  bool exists = false;
  if (node) {
    if (attribs) {
      if (member.isInteger()) {
        int64_t nodendx = 0;

        while (attr && nodendx <= member.toInt64()) {
          if ((!test || !xmlStrcmp(attr->name, sxe->iter.name)) &&
              match_ns(sxe, (xmlNodePtr) attr, sxe->iter.nsprefix,
                       sxe->iter.isprefix)) {
            if (nodendx == member.toInt64()) {
              exists = true;
              break;
            }
            nodendx++;
          }
          attr = attr->next;
        }
      } else {
        while (attr) {
          if ((!test || !xmlStrcmp(attr->name, sxe->iter.name)) &&
              !xmlStrcmp(attr->name, (xmlChar*)member.toString().data()) &&
              match_ns(sxe, (xmlNodePtr) attr, sxe->iter.nsprefix,
                       sxe->iter.isprefix)) {
            exists = true;
            break;
          }

          attr = attr->next;
        }
      }
      if (exists && check_empty == 1 &&
          (!attr->children || !attr->children->content ||
           !attr->children->content[0] ||
           !xmlStrcmp(attr->children->content, (const xmlChar*)"0")) ) {
        /* Attribute with no content in it's text node */
        exists = false;
      }
    }

    if (elements) {
      if (member.isInteger()) {
        if (sxe->iter.type == SXE_ITER_CHILD) {
          node = php_sxe_get_first_node(sxe, node);
        }
        node = sxe_get_element_by_offset(sxe, member.toInt64(), node, nullptr);
      }
      else {
        node = node->children;
        while (node) {
          xmlNodePtr nnext;
          nnext = node->next;
          if ((node->type == XML_ELEMENT_NODE) &&
              !xmlStrcmp(node->name, (xmlChar*)member.toString().data())) {
            break;
          }
          node = nnext;
        }
      }
      if (node) {
        exists = true;
        if (check_empty == true &&
            (!node->children || (node->children->type == XML_TEXT_NODE &&
                                 !node->children->next &&
                                 (!node->children->content ||
                                  !node->children->content[0] ||
                                  !xmlStrcmp(node->children->content,
                                             (const xmlChar*)"0"))))) {
          exists = false;
        }
      }
    }
  }

  return exists;
}

static inline String sxe_xmlNodeListGetString(xmlDocPtr doc, xmlNodePtr list,
                                              bool inLine) {
  xmlChar* tmp = xmlNodeListGetString(doc, list, inLine);
  if (tmp) {
    String ret = String((char*)tmp);
    xmlFree(tmp);
    return ret;
  } else {
    return empty_string();
  }
}

static Variant _get_base_node_value(SimpleXMLElement* sxe_ref,
                                    xmlNodePtr node, xmlChar* nsprefix,
                                    bool isprefix) {
  if (node->children &&
      node->children->type == XML_TEXT_NODE &&
      !xmlIsBlankNode(node->children)) {
    xmlChar* contents = xmlNodeListGetString(node->doc, node->children, 1);
    if (contents) {
      String obj = String((char*)contents);
      xmlFree(contents);
      return obj;
    }
  } else {
    auto sxeRefObj = Native::object<SimpleXMLElement>(sxe_ref);
    Object obj = create_object(sxeRefObj->getClassName(), Array(), false);
    auto subnode = Native::data<SimpleXMLElement>(obj.get());
    if (nsprefix && *nsprefix) {
      subnode->iter.nsprefix = xmlStrdup((xmlChar*)nsprefix);
      subnode->iter.isprefix = isprefix;
    }
    subnode->node = libxml_register_node(node);
    return obj;
  }
  return init_null();
}

static void sxe_properties_add(Array& rv, char* name, const Variant& value) {
  String sName = String(name);
  if (rv.exists(sName)) {
    Variant existVal = rv[sName];
    if (existVal.isArray()) {
      Array arr = existVal.toArray();
      arr.append(value);
      rv.set(sName, arr);
    } else {
      Array arr = make_vec_array(existVal, value);
      rv.set(sName, arr);
    }
  } else {
    rv.set(sName, value);
  }
}

const static StaticString s_atAttributes{"@attributes"};

static void sxe_get_prop_hash(SimpleXMLElement* sxe, bool is_debug,
                              Array& rv, bool isBoolCast = false) {
  rv.clear();

  Object iter_data;
  bool use_iter = false;
  xmlNodePtr node = sxe->nodep();
  if (!node) {
    return;
  }
  if (is_debug || sxe->iter.type != SXE_ITER_CHILD) {
    if (sxe->iter.type == SXE_ITER_ELEMENT) {
      node = php_sxe_get_first_node(sxe, node);
    }
    if (!node || node->type != XML_ENTITY_DECL) {
      xmlAttrPtr attr = node ? (xmlAttrPtr)node->properties : nullptr;
      Array zattr = Array::CreateDict();
      bool test = sxe->iter.name && sxe->iter.type == SXE_ITER_ATTRLIST;
      while (attr) {
        if ((!test || !xmlStrcmp(attr->name, sxe->iter.name)) &&
            match_ns(sxe, (xmlNodePtr)attr, sxe->iter.nsprefix,
                     sxe->iter.isprefix)) {
          zattr.set(String((char*)attr->name),
                    sxe_xmlNodeListGetString(
                      sxe->docp(),
                      attr->children,
                      1));
        }
        attr = attr->next;
      }
      if (zattr.size()) {
        rv.set(s_atAttributes, zattr);
      }
    }
  }

  node = sxe->nodep();
  node = php_sxe_get_first_node(sxe, node);

  if (node && sxe->iter.type != SXE_ITER_ATTRLIST) {
    if (node->type == XML_ATTRIBUTE_NODE) {
      rv.append(sxe_xmlNodeListGetString(node->doc, node->children, 1));
      node = nullptr;
    } else if (sxe->iter.type != SXE_ITER_CHILD) {
      if (sxe->iter.type == SXE_ITER_NONE || !node->children ||
          !node->parent ||
          node->children->next ||
          node->children->children ||
          node->parent->children == node->parent->last) {
        node = node->children;
      } else {
        iter_data = sxe->iter.data;
        sxe->iter.data.reset();

        node = php_sxe_reset_iterator(sxe, false);

        use_iter = true;
      }
    }

    char *name  = nullptr;
    Variant value;
    while (node) {
      if (node->children != nullptr || node->prev != nullptr ||
          node->next != nullptr) {
        SKIP_TEXT(node);
      } else {
        if (node->type == XML_TEXT_NODE) {
          const xmlChar* cur = node->content;

          if (*cur != 0) {
            rv.append(sxe_xmlNodeListGetString(node->doc, node, 1));
          }
          goto next_iter;
        }
      }

      if (node->type == XML_ELEMENT_NODE &&
          (!match_ns(sxe, node, sxe->iter.nsprefix, sxe->iter.isprefix))) {
        goto next_iter;
      }

      name = (char*)node->name;
      if (!name) {
        goto next_iter;
      }

      value = _get_base_node_value(sxe, node, sxe->iter.nsprefix,
                                   sxe->iter.isprefix);
      if (use_iter) {
        rv.append(value);
      } else {
        sxe_properties_add(rv, name, value);
      }
      if (isBoolCast) break;
next_iter:
      if (use_iter) {
        node = php_sxe_iterator_fetch(sxe, node->next, 0);
      } else {
        node = node->next;
      }
    }
  }

  if (use_iter) {
    sxe->iter.data = iter_data;
  }
}

Array SimpleXMLElement_darrayCast(const ObjectData* obj) {
  auto sxe = Native::data<SimpleXMLElement>(const_cast<ObjectData*>(obj));
  Array properties = Array::CreateDict();
  sxe_get_prop_hash(sxe, true, properties);
  return properties;
}

Variant SimpleXMLElement_objectCast(const ObjectData* obj, DataType type) {
  assertx(!isArrayLikeType(type));
  assertx(obj->instanceof(SimpleXMLElementLoader::classof()));
  auto sxe = Native::data<SimpleXMLElement>(const_cast<ObjectData*>(obj));
  if (type == KindOfBoolean) {
    xmlNodePtr node = php_sxe_get_first_node(sxe, nullptr);
    if (node) return true;
    Array properties = Array::CreateDict();
    sxe_get_prop_hash(sxe, true, properties, true);
    return properties.size() != 0;
  }

  xmlChar* contents = nullptr;
  if (sxe->iter.type != SXE_ITER_NONE) {
    xmlNodePtr node = php_sxe_get_first_node(sxe, nullptr);
    if (node) {
      contents = xmlNodeListGetString(sxe->docp(), node->children, 1);
    }
  } else {
    xmlDocPtr doc = sxe->docp();
    if (!sxe->nodep()) {
      if (doc) {
        sxe->node = libxml_register_node(xmlDocGetRootElement(doc));
      }
    }

    if (sxe->nodep()) {
      if (sxe->nodep()->children) {
        contents = xmlNodeListGetString(doc, sxe->nodep()->children, 1);
      }
    }
  }

  String ret = String((char*)contents);
  if (contents) {
    xmlFree(contents);
  }

  switch (type) {
    case KindOfString: return ret;
    case KindOfInt64:  return ret.toInt64();
    case KindOfDouble: return ret.toDouble();
    default:           return init_null();
  }
}

static bool sxe_prop_dim_write(SimpleXMLElement* sxe, const Variant& member,
                               const Variant& value, bool elements, bool attribs,
                               xmlNodePtr* pnewnode) {
  xmlNodePtr node = sxe->nodep();

  if (member.isNull() || member.isInteger()) {
    if (sxe->iter.type != SXE_ITER_ATTRLIST) {
      attribs = false;
      elements = true;
    } else if (member.isNull()) {
      /* This happens when the user did: $sxe[] = $value
       * and could also be E_PARSE, but we use this only during parsing
       * and this is during runtime.
       */
      raise_error("Cannot create unnamed attribute");
      return false;
    }
  } else {
    if (member.toString().empty()) {
      raise_warning("Cannot write or create unnamed %s",
                    attribs ? "attribute" : "element");
      return false;
    }
  }

  bool retval = true;
  xmlAttrPtr attr   = nullptr;
  xmlNodePtr mynode = nullptr;
  bool test = false;
  if (sxe->iter.type == SXE_ITER_ATTRLIST) {
    attribs = true;
    elements = false;
    node = php_sxe_get_first_node(sxe, node);
    attr = (xmlAttrPtr)node;
    test = sxe->iter.name != nullptr;
  } else if (sxe->iter.type != SXE_ITER_CHILD) {
    mynode = node;
    node = php_sxe_get_first_node(sxe, node);
    attr = node ? node->properties : nullptr;
    test = false;
    if (member.isNull() && node && node->parent &&
        node->parent->type == XML_DOCUMENT_NODE) {
      /* This happens when the user did: $sxe[] = $value
       * and could also be E_PARSE, but we use this only during parsing
       * and this is during runtime.
       */
      raise_error("Cannot create unnamed attribute");
      return false;
    }
    if (attribs && !node && sxe->iter.type == SXE_ITER_ELEMENT) {
      node = xmlNewChild(mynode, mynode->ns, sxe->iter.name, nullptr);
      attr = node->properties;
    }
  }

  mynode = node;

  if (!(value.isString() || value.isInteger() || value.isBoolean() ||
      value.isDouble() || value.isNull() || value.isObject())) {
    raise_warning("It is not yet possible to assign complex types to %s",
                  attribs ? "attributes" : "properties");
    return false;
  }

  xmlNodePtr newnode = nullptr;
  if (node) {
    int64_t nodendx = 0;
    int64_t counter = 0;
    bool is_attr = false;
    if (attribs) {
      if (member.isInteger()) {
        while (attr && nodendx <= member.toInt64()) {
          if ((!test || !xmlStrcmp(attr->name, sxe->iter.name)) &&
              match_ns(sxe, (xmlNodePtr) attr, sxe->iter.nsprefix,
                       sxe->iter.isprefix)) {
            if (nodendx == member.toInt64()) {
              is_attr = true;
              ++counter;
              break;
            }
            nodendx++;
          }
          attr = attr->next;
        }
      } else {
        while (attr) {
          if ((!test || !xmlStrcmp(attr->name, sxe->iter.name)) &&
              !xmlStrcmp(attr->name, (xmlChar*)member.toString().data()) &&
              match_ns(sxe, (xmlNodePtr) attr, sxe->iter.nsprefix,
                       sxe->iter.isprefix)) {
            is_attr = true;
            ++counter;
            break;
          }
          attr = attr->next;
        }
      }

    }

    long cnt = 0;
    if (elements) {
      if (member.isNull() || member.isInteger()) {
        if (node->type == XML_ATTRIBUTE_NODE) {
          raise_error("Cannot create duplicate attribute");
          return false;
        }

        if (sxe->iter.type == SXE_ITER_NONE) {
          newnode = node;
          ++counter;
          if (!member.isNull() && member.toInt64() > 0) {
            raise_warning("Cannot add element %s number %" PRId64 " when "
                          "only 0 such elements exist", mynode->name,
                          member.toInt64());
            retval = false;
          }
        } else if (!member.isNull()) {
          newnode =
            sxe_get_element_by_offset(sxe, member.toInt64(), node, &cnt);
          if (newnode) {
            ++counter;
          }
        }
      } else {
        node = node->children;
        while (node) {
          SKIP_TEXT(node)

          if (!xmlStrcmp(node->name, (xmlChar*)member.toString().data())) {
            newnode = node;
            ++counter;
          }

next_iter:
          node = node->next;
        }
      }
    }

    if (counter == 1) {
      if (is_attr) {
        newnode = (xmlNodePtr) attr;
      }
      if (!value.isNull()) {
        xmlNodePtr tempnode;
        while ((tempnode = (xmlNodePtr) newnode->children)) {
          libxml_register_node(tempnode)->unlink();
        }
        change_node_zval(newnode, value);
      }
    } else if (counter > 1) {
      raise_warning("Cannot assign to an array of nodes "
                    "(duplicate subnodes or attr detected)");
      retval = false;
    } else if (elements) {
      if (!node) {
        if (member.isNull() || member.isInteger()) {
          newnode =
            xmlNewTextChild(
              mynode->parent, mynode->ns, mynode->name,
              !value.isNull() ? (xmlChar*)value.toString().data() : nullptr);
        } else {
          newnode =
            xmlNewTextChild(
              mynode, mynode->ns, (xmlChar*)member.toString().data(),
              !value.isNull() ? (xmlChar*)value.toString().data() : nullptr);
        }
      } else if (member.isNull() || member.isInteger()) {
        if (!member.isNull() && cnt < member.toInt64()) {
          raise_warning("Cannot add element %s number %" PRId64 " when "
                        "only %ld such elements exist", mynode->name,
                        member.toInt64(), cnt);
          retval = false;
        }
        newnode = xmlNewTextChild(mynode->parent, mynode->ns, mynode->name,
                                  !value.isNull() ?
                                  (xmlChar*)value.toString().data() : nullptr);
      }
    } else if (attribs) {
      if (member.isInteger()) {
        raise_warning("Cannot change attribute number %" PRId64 " when "
                      "only %" PRId64 " attributes exist", member.toInt64(),
                      nodendx);
        retval = false;
      } else {
        newnode = (xmlNodePtr)xmlNewProp(node,
                                         (xmlChar*)member.toString().data(),
                                         !value.isNull() ?
                                          (xmlChar*)value.toString().data() :
                                          nullptr);
      }
    }
  }

  if (pnewnode) {
    *pnewnode = newnode;
  }
  return retval;
}

///////////////////////////////////////////////////////////////////////////////
// SimpleXML

static const Class* class_from_name(const String& class_name,
                                    const char* callee) {
  const Class* cls;
  if (!class_name.empty()) {
    cls = Class::load(class_name.get());
    if (!cls) {
      raise_invalid_argument_warning("class not found: %s", class_name.data());
      return nullptr;
    }
    if (!cls->classof(SimpleXMLElementLoader::classof())) {
      raise_invalid_argument_warning(
        "%s() expects parameter 2 to be a class name "
        "derived from SimpleXMLElement, '%s' given",
        callee,
        class_name.data());
      return nullptr;
    }
  } else {
    cls = SimpleXMLElementLoader::classof();
  }
  return cls;
}

const StaticString s_DOMNode("DOMNode");

static Variant HHVM_FUNCTION(simplexml_import_dom,
                             const Object& node,
                             const String& class_name) {
  if (!node->instanceof(s_DOMNode)) {
    raise_warning("Invalid Nodetype to import");
    return init_null();
  }
  auto domnode = Native::data<DOMNode>(node);
  xmlNodePtr nodep = domnode->nodep();

  if (nodep) {
    if (nodep->doc == nullptr) {
      raise_warning("Imported Node must have associated Document");
      return init_null();
    }
    if (nodep->type == XML_DOCUMENT_NODE ||
        nodep->type == XML_HTML_DOCUMENT_NODE) {
      nodep = xmlDocGetRootElement((xmlDocPtr) nodep);
    }
  }

  if (nodep && nodep->type == XML_ELEMENT_NODE) {
    auto cls = class_from_name(class_name, "simplexml_import_dom");
    if (!cls) {
      return init_null();
    }
    Object obj = create_object(cls->nameStr(), Array(), false);
    auto sxe = Native::data<SimpleXMLElement>(obj.get());
    sxe->node = libxml_register_node(nodep);
    return obj;
  } else {
    raise_warning("Invalid Nodetype to import");
    return init_null();
  }
  return false;
}

static Variant HHVM_FUNCTION(simplexml_load_string,
  const String& data,
  const String& class_name /* = "SimpleXMLElement" */,
  int64_t options /* = 0 */,
  const String& ns /* = "" */,
  bool is_prefix /* = false */) {
  SYNC_VM_REGS_SCOPED();
  auto cls = class_from_name(class_name, "simplexml_load_string");
  if (!cls) {
    return init_null();
  }

  xmlDocPtr doc = xmlReadMemory(data.data(), data.size(), nullptr,
                                     nullptr, options);
  if (!doc) {
    return false;
  }

  Object obj = create_object(cls->nameStr(), Array(), false);
  auto sxe = Native::data<SimpleXMLElement>(obj.get());
  sxe->node = libxml_register_node(xmlDocGetRootElement(doc));
  sxe->iter.nsprefix = ns.size() ? xmlStrdup((xmlChar*)ns.data()) : nullptr;
  sxe->iter.isprefix = is_prefix;
  return obj;
}

static Variant
HHVM_FUNCTION(simplexml_load_file, const String& filename,
              const String& class_name /* = "SimpleXMLElement" */,
              int64_t /*options*/ /* = 0 */, const String& ns /* = "" */,
              bool is_prefix /* = false */) {
  SYNC_VM_REGS_SCOPED();
  auto cls = class_from_name(class_name, "simplexml_load_file");
  if (!cls) {
    return init_null();
  }

  auto stream = File::Open(filename, "rb");
  if (!stream || stream->isInvalid()) return false;

  xmlDocPtr doc = nullptr;

  // The XML context is also deleted in this function, so the ownership
  // of the File is kept locally in 'stream'. The libxml_streams_IO_nop_close
  // callback does nothing.
  xmlParserCtxtPtr ctxt = xmlCreateIOParserCtxt(nullptr, nullptr,
                                                libxml_streams_IO_read,
                                                libxml_streams_IO_nop_close,
                                                &stream,
                                                XML_CHAR_ENCODING_NONE);
  if (ctxt == nullptr) return false;
  SCOPE_EXIT { xmlFreeParserCtxt(ctxt); };

  if (ctxt->directory == nullptr) {
    ctxt->directory = xmlParserGetDirectory(filename.c_str());
  }
  xmlParseDocument(ctxt);
  if (ctxt->wellFormed) {
    doc = ctxt->myDoc;
  } else {
    xmlFreeDoc(ctxt->myDoc);
    ctxt->myDoc = nullptr;
    return false;
  }

  Object obj = create_object(cls->nameStr(), Array(), false);
  auto sxe = Native::data<SimpleXMLElement>(obj.get());
  sxe->node = libxml_register_node(xmlDocGetRootElement(doc));
  sxe->iter.nsprefix = ns.size() ? xmlStrdup((xmlChar*)ns.data()) : nullptr;
  sxe->iter.isprefix = is_prefix;
  return obj;
}

///////////////////////////////////////////////////////////////////////////////
// SimpleXMLElement

static void HHVM_METHOD(SimpleXMLElement, __construct,
                        const String& data,
                        int64_t options /* = 0 */,
                        bool data_is_url /* = false */,
                        const String& ns /* = "" */,
                        bool is_prefix /* = false */) {
  SYNC_VM_REGS_SCOPED();
  xmlDocPtr docp = data_is_url ?
    xmlReadFile(data.data(), nullptr, options) :
    xmlReadMemory(data.data(), data.size(), nullptr, nullptr, options);
  if (!docp) {
    SystemLib::throwExceptionObject("String could not be parsed as XML");
  }

  auto sxe = Native::data<SimpleXMLElement>(this_);
  sxe->iter.nsprefix = !ns.empty() ? xmlStrdup((xmlChar*)ns.data()) : nullptr;
  sxe->iter.isprefix = is_prefix;
  sxe->node = libxml_register_node(xmlDocGetRootElement(docp));
}

static Variant HHVM_METHOD(SimpleXMLElement, xpath, const String& path) {
  auto data = Native::data<SimpleXMLElement>(this_);
  if (data->iter.type == SXE_ITER_ATTRLIST) {
    return init_null(); // attributes don't have attributes
  }

  if (!data->xpath) {
    data->xpath = xmlXPathNewContext(data->docp());
  }
  if (!data->nodep()) {
    data->node = libxml_register_node(xmlDocGetRootElement(data->docp()));
  }

  auto nodeptr = php_sxe_get_first_node(data, data->nodep());
  data->xpath->node = nodeptr;

  xmlNsPtr* ns = xmlGetNsList(data->docp(), nodeptr);
  int64_t nsnbr = 0;
  if (ns != nullptr) {
    while (ns[nsnbr] != nullptr) {
      nsnbr++;
    }
  }

  auto& xpath = data->xpath;
  xpath->namespaces = ns;
  xpath->nsNr = nsnbr;

  xmlXPathObjectPtr retval = xmlXPathEval((xmlChar*)path.data(), xpath);
  if (ns != nullptr) {
    xmlFree(ns);
    xpath->namespaces = nullptr;
    xpath->nsNr = 0;
  }

  if (!retval) {
    return false;
  }

  xmlNodeSetPtr result = retval->nodesetval;

  Array ret = Array::CreateVec();
  if (result != nullptr) {
    for (int64_t i = 0; i < result->nodeNr; ++i) {
      nodeptr = result->nodeTab[i];
      if (nodeptr->type == XML_TEXT_NODE ||
          nodeptr->type == XML_ELEMENT_NODE ||
          nodeptr->type == XML_ATTRIBUTE_NODE) {
        /**
         * Detect the case where the last selector is text(), simplexml
         * always accesses the text() child by default, therefore we assign
         * to the parent node.
         */
        Object obj;
        if (nodeptr->type == XML_TEXT_NODE) {
          obj = _node_as_zval(data, nodeptr->parent, SXE_ITER_NONE, nullptr,
                              nullptr, false);
        } else if (nodeptr->type == XML_ATTRIBUTE_NODE) {
          obj = _node_as_zval(data, nodeptr->parent, SXE_ITER_ATTRLIST,
                              (char*)nodeptr->name, nodeptr->ns ?
                                (xmlChar*)nodeptr->ns->href : nullptr, false);
        } else {
          obj = _node_as_zval(data, nodeptr, SXE_ITER_NONE, nullptr, nullptr,
                              false);
        }
        if (!obj.isNull()) {
          ret.append(obj);
        }
      }
    }
  }

  xmlXPathFreeObject(retval);
  return ret;
}

static bool HHVM_METHOD(SimpleXMLElement, registerXPathNamespace,
                        const String& prefix, const String& ns) {
  auto data = Native::data<SimpleXMLElement>(this_);
  if (!data->xpath) {
    data->xpath = xmlXPathNewContext(data->docp());
  }

  if (xmlXPathRegisterNs(data->xpath,
                         (xmlChar*)prefix.data(),
                         (xmlChar*)ns.data()) != 0) {
    return false;
  }
  return true;
}

static Variant HHVM_METHOD(SimpleXMLElement, asXML,
                           const String& filename /* = "" */) {
  auto data = Native::data<SimpleXMLElement>(this_);
  xmlNodePtr node = data->nodep();
  xmlOutputBufferPtr outbuf = nullptr;

  if (filename.size()) {
    node = php_sxe_get_first_node(data, node);

    if (node) {
      xmlDocPtr doc = data->docp();
      if (node->parent && (XML_DOCUMENT_NODE == node->parent->type)) {
        int bytes;
        bytes = xmlSaveFile(filename.data(), doc);
        if (bytes == -1) {
          return false;
        } else {
          return true;
        }
      } else {
        outbuf = xmlOutputBufferCreateFilename(filename.data(), nullptr, 0);

        if (outbuf == nullptr) {
          return false;
        }

        xmlNodeDumpOutput(outbuf, doc, node, 0, 0, nullptr);
        xmlOutputBufferClose(outbuf);
        return true;
      }
    } else {
      return false;
    }
  }

  node = php_sxe_get_first_node(data, node);

  if (node) {
    xmlDocPtr doc = data->docp();
    if (node->parent && (XML_DOCUMENT_NODE == node->parent->type)) {
      xmlChar* strval;
      int strval_len;
      xmlDocDumpMemoryEnc(doc, &strval, &strval_len,
                          (const char*)doc->encoding);
      if (!strval) {
        return false;
      }
      String ret = String((char*)strval);
      xmlFree(strval);
      return ret;
    } else {
      /* Should we be passing encoding information instead of nullptr? */
      outbuf = xmlAllocOutputBuffer(nullptr);

      if (outbuf == nullptr) {
        return false;
      }

      xmlNodeDumpOutput(outbuf, doc, node, 0, 0,
                        (const char*)doc->encoding);
      xmlOutputBufferFlush(outbuf);

      char* str = nullptr;
#ifdef LIBXML2_NEW_BUFFER
      str = (char*)xmlOutputBufferGetContent(outbuf);
#else
      str = (char*)outbuf->buffer->content;
#endif
      if (!str) {
        return false;
      }
      String ret = String(str);
      xmlOutputBufferClose(outbuf);
      return ret;
    }
  } else {
    return false;
  }
  return false;
}

static Array HHVM_METHOD(SimpleXMLElement, getNamespaces,
                         bool recursive /* = false */) {
  auto data = Native::data<SimpleXMLElement>(this_);
  Array ret = Array::CreateDict();
  xmlNodePtr node = data->nodep();
  node = php_sxe_get_first_node(data, node);
  if (node) {
    if (node->type == XML_ELEMENT_NODE) {
      sxe_add_namespaces(data, node, recursive, ret);
    } else if (node->type == XML_ATTRIBUTE_NODE && node->ns) {
      sxe_add_namespace_name(ret, node->ns);
    }
  }
  return ret;
}

static Array HHVM_METHOD(SimpleXMLElement, getDocNamespaces,
                         bool recursive /* = false */,
                         bool from_root /* = true */) {
  auto data = Native::data<SimpleXMLElement>(this_);
  xmlNodePtr node =
    from_root ? xmlDocGetRootElement(data->docp())
              : data->nodep();
  Array ret = Array::CreateDict();
  sxe_add_registered_namespaces(data, node, recursive, ret);
  return ret;
}

static Variant HHVM_METHOD(SimpleXMLElement, children,
                           const String& ns = empty_string(),
                           bool is_prefix = false) {
  auto data = Native::data<SimpleXMLElement>(this_);
  if (data->iter.type == SXE_ITER_ATTRLIST) {
    return init_null(); /* attributes don't have attributes */
  }

  xmlNodePtr node = data->nodep();
  node = php_sxe_get_first_node(data, node);
  return _node_as_zval(data, node, SXE_ITER_CHILD, nullptr,
                       (xmlChar*)ns.data(), is_prefix);
}

static String HHVM_METHOD(SimpleXMLElement, getName) {
  auto data = Native::data<SimpleXMLElement>(this_);
  xmlNodePtr node = data->nodep();
  node = php_sxe_get_first_node(data, node);
  if (node) {
    return String((char*)node->name);
  }
  return empty_string();
}

static Object HHVM_METHOD(SimpleXMLElement, attributes,
                          const String& ns /* = "" */,
                          bool is_prefix /* = false */) {
  auto data = Native::data<SimpleXMLElement>(this_);
  if (data->iter.type == SXE_ITER_ATTRLIST) {
    return Object(); /* attributes don't have attributes */
  }

  xmlNodePtr node = data->nodep();
  node = php_sxe_get_first_node(data, node);
  return _node_as_zval(data, node, SXE_ITER_ATTRLIST, nullptr,
                       (xmlChar*)ns.data(), is_prefix);
}

static Variant HHVM_METHOD(SimpleXMLElement, addChild,
                           const String& qname,
                           const String& value /* = null_string */,
                           const Variant& ns /* = null */) {
  if (qname.empty()) {
    raise_warning("Element name is required");
    return init_null();
  }

  auto data = Native::data<SimpleXMLElement>(this_);
  xmlNodePtr node = data->nodep();

  if (data->iter.type == SXE_ITER_ATTRLIST) {
    raise_warning("Cannot add element to attributes");
    return init_null();
  }

  node = php_sxe_get_first_node(data, node);

  if (node == nullptr) {
    raise_warning("Cannot add child. "
                  "Parent is not a permanent member of the XML tree");
    return init_null();
  }

  xmlChar* prefix = nullptr;
  xmlChar* localname = xmlSplitQName2((xmlChar*)qname.data(), &prefix);
  if (localname == nullptr) {
    localname = xmlStrdup((xmlChar*)qname.data());
  }

  xmlNodePtr newnode = xmlNewChild(node, nullptr, localname,
                                   (xmlChar*)value.data());

  xmlNsPtr nsptr = nullptr;
  if (!ns.isNull()) {
    const String& ns_ = ns.toString();
    if (ns_.empty()) {
      newnode->ns = nullptr;
      nsptr = xmlNewNs(newnode, (xmlChar*)ns_.data(), prefix);
    } else {
      nsptr = xmlSearchNsByHref(node->doc, node, (xmlChar*)ns_.data());
      if (nsptr == nullptr) {
        nsptr = xmlNewNs(newnode, (xmlChar*)ns_.data(), prefix);
      }
      newnode->ns = nsptr;
    }
  }

  Object ret = _node_as_zval(data, newnode, SXE_ITER_NONE, (char*)localname,
                             prefix, false);

  xmlFree(localname);
  if (prefix != nullptr) {
    xmlFree(prefix);
  }
  return ret;
}

static void HHVM_METHOD(SimpleXMLElement, addAttribute,
                        const String& qname,
                        const String& value /* = null_string */,
                        const String& ns /* = null_string */) {
  if (qname.size() == 0) {
    raise_warning("Attribute name is required");
    return;
  }

  auto data = Native::data<SimpleXMLElement>(this_);
  xmlNodePtr node = data->nodep();
  node = php_sxe_get_first_node(data, node);

  if (node && node->type != XML_ELEMENT_NODE) {
    node = node->parent;
  }

  if (node == nullptr) {
    raise_warning("Unable to locate parent Element");
    return;
  }

  xmlChar* prefix = nullptr;
  xmlChar* localname = xmlSplitQName2((xmlChar*)qname.data(), &prefix);
  if (localname == nullptr) {
    if (ns.size() > 0) {
      if (prefix != nullptr) {
        xmlFree(prefix);
      }
      raise_warning("Attribute requires prefix for namespace");
      return;
    }
    localname = xmlStrdup((xmlChar*)qname.data());
  }

  xmlAttrPtr attrp = xmlHasNsProp(node, localname, (xmlChar*)ns.data());
  if (attrp != nullptr && attrp->type != XML_ATTRIBUTE_DECL) {
    xmlFree(localname);
    if (prefix != nullptr) {
      xmlFree(prefix);
    }
    raise_warning("Attribute already exists");
    return;
  }

  xmlNsPtr nsptr = nullptr;
  if (ns.size()) {
    nsptr = xmlSearchNsByHref(node->doc, node, (xmlChar*)ns.data());
    if (nsptr == nullptr) {
      nsptr = xmlNewNs(node, (xmlChar*)ns.data(), prefix);
    }
  }

  attrp = xmlNewNsProp(node, nsptr, localname, (xmlChar*)value.data());

  xmlFree(localname);
  if (prefix != nullptr) {
    xmlFree(prefix);
  }
}

static String HHVM_METHOD(SimpleXMLElement, __toString) {
  return SimpleXMLElement_objectCast(this_, KindOfString).toString();
}

struct SimpleXMLElementPropHandler: Native::BasePropHandler {
  static Variant getProp(const Object& this_, const String& name) {
    auto data = Native::data<SimpleXMLElement>(this_.get());
    return sxe_prop_dim_read(data, name, true, false);
  }

  static Variant setProp(const Object& this_,
                         const String& name,
                         const Variant& value) {
    auto data = Native::data<SimpleXMLElement>(this_.get());
    return sxe_prop_dim_write(data, name, value, true, false, nullptr);
    return true;
  }

  static Variant issetProp(const Object& this_, const String& name) {
    auto data = Native::data<SimpleXMLElement>(this_.get());
    return sxe_prop_dim_exists(data, name, false, true, false);
  }

  static Variant unsetProp(const Object& this_, const String& name) {
    auto data = Native::data<SimpleXMLElement>(this_.get());
    sxe_prop_dim_delete(data, name, true, false);
    return true;
  }

  static bool isPropSupported(const String&, const String&) {
    // TODO: we really ought to pull out checking of whether a prop is supported
    // to here, but it's currently very entagled in the normal functionality
    // so defer that tech debt to later :p
    return true;
  }
};

static int64_t HHVM_METHOD(SimpleXMLElement, count) {
  auto data = Native::data<SimpleXMLElement>(this_);
  return php_sxe_count_elements_helper(data);
}

///////////////////////////////////////////////////////////////////////////////
// ArrayAccess

static bool HHVM_METHOD(SimpleXMLElement, offsetExists,
                        const Variant& index) {
  auto data = Native::data<SimpleXMLElement>(this_);
  return sxe_prop_dim_exists(data, index, false, false, true);
}

static Variant HHVM_METHOD(SimpleXMLElement, offsetGet,
                           const Variant& index) {
  auto data = Native::data<SimpleXMLElement>(this_);
  return sxe_prop_dim_read(data, index, false, true);
}

static void HHVM_METHOD(SimpleXMLElement, offsetSet,
                        const Variant& index, const Variant& newvalue) {
  auto data = Native::data<SimpleXMLElement>(this_);
  sxe_prop_dim_write(data, index, newvalue, false, true, nullptr);
}

static void HHVM_METHOD(SimpleXMLElement, offsetUnset,
                        const Variant& index) {
  auto data = Native::data<SimpleXMLElement>(this_);
  sxe_prop_dim_delete(data, index, false, true);
}

///////////////////////////////////////////////////////////////////////////////
// Iterator

static void HHVM_METHOD(SimpleXMLElementIterator, __construct,
                        const Variant& sxe) {
  if (sxe.isObject()) {
    Native::data<SimpleXMLElementIterator>(this_)->setSxe(sxe.toObject());
  }
}

static Object HHVM_METHOD(SimpleXMLElementIterator, current) {
  return Native::data<SimpleXMLElementIterator>(this_)->sxe()->iter.data;
}

static Variant HHVM_METHOD(SimpleXMLElementIterator, key) {
  auto sxe = Native::data<SimpleXMLElementIterator>(this_)->sxe();
  Object curobj = sxe->iter.data;
  xmlNodePtr curnode = curobj.isNull()
    ? nullptr
    : Native::data<SimpleXMLElement>(curobj.get())->nodep();
  if (curnode) {
    return String((char*)curnode->name);
  } else {
    return init_null();
  }
}

static void HHVM_METHOD(SimpleXMLElementIterator, next) {
  auto sxe = Native::data<SimpleXMLElementIterator>(this_)->sxe();
  php_sxe_move_forward_iterator(sxe);
}

static void HHVM_METHOD(SimpleXMLElementIterator, rewind) {
  auto sxe = Native::data<SimpleXMLElementIterator>(this_)->sxe();
  php_sxe_reset_iterator(sxe, true);
}

static bool HHVM_METHOD(SimpleXMLElementIterator, valid) {
  auto sxe = Native::data<SimpleXMLElementIterator>(this_)->sxe();
  return !sxe->iter.data.isNull();
}

///////////////////////////////////////////////////////////////////////////////
// SimpleXMLIterator

static Object HHVM_METHOD(SimpleXMLIterator, current) {
  auto data = Native::data<SimpleXMLIterator>(this_);
  return data->iter.data;
}

static Variant HHVM_METHOD(SimpleXMLIterator, key) {
  auto data = Native::data<SimpleXMLIterator>(this_);
  Object curobj = data->iter.data;
  if (curobj.isNull()) {
    return init_null();
  }

  assertx(curobj->instanceof(SimpleXMLElementLoader::classof()));
  auto curnode = Native::data<SimpleXMLElement>(curobj.get())->nodep();
  return String((char*)curnode->name);
}

static void HHVM_METHOD(SimpleXMLIterator, next) {
  auto data = Native::data<SimpleXMLIterator>(this_);
  php_sxe_move_forward_iterator(data);
}

static void HHVM_METHOD(SimpleXMLIterator, rewind) {
  auto data = Native::data<SimpleXMLIterator>(this_);
  php_sxe_reset_iterator(data, true);
}

static bool HHVM_METHOD(SimpleXMLIterator, valid) {
  auto data = Native::data<SimpleXMLIterator>(this_);
  return !data->iter.data.isNull();
}

static Variant HHVM_METHOD(SimpleXMLIterator, getChildren) {
  auto data = Native::data<SimpleXMLIterator>(this_);
  auto current = data->iter.data;
  if (current.isNull()) {
    return init_null();
  }
  assertx(current->instanceof(SimpleXMLElementLoader::classof()));
  return HHVM_MN(SimpleXMLElement, children)(current.get());
}

static bool HHVM_METHOD(SimpleXMLIterator, hasChildren) {
  auto children = HHVM_MN(SimpleXMLIterator, getChildren)(this_);
  if (!children.isObject()) {
    return false;
  }
  auto od = children.toObject().get();
  assertx(od->instanceof(SimpleXMLElementLoader::classof()));
  return HHVM_MN(SimpleXMLElement, count)(od) > 0;
}

///////////////////////////////////////////////////////////////////////////////

static struct SimpleXMLExtension : Extension {
  SimpleXMLExtension(): Extension("simplexml", "1.0", NO_ONCALL_YET) {}

  void moduleInit() override {
    HHVM_FE(simplexml_import_dom);
    HHVM_FE(simplexml_load_string);
    HHVM_FE(simplexml_load_file);

    /* SimpleXMLElement */
    HHVM_ME(SimpleXMLElement, __construct);
    HHVM_ME(SimpleXMLElement, xpath);
    HHVM_ME(SimpleXMLElement, registerXPathNamespace);
    HHVM_ME(SimpleXMLElement, asXML);
    HHVM_ME(SimpleXMLElement, getNamespaces);
    HHVM_ME(SimpleXMLElement, getDocNamespaces);
    HHVM_ME(SimpleXMLElement, children);
    HHVM_ME(SimpleXMLElement, getName);
    HHVM_ME(SimpleXMLElement, attributes);
    HHVM_ME(SimpleXMLElement, addChild);
    HHVM_ME(SimpleXMLElement, addAttribute);
    HHVM_ME(SimpleXMLElement, __toString);
    HHVM_ME(SimpleXMLElement, count);
    HHVM_ME(SimpleXMLElement, offsetExists);
    HHVM_ME(SimpleXMLElement, offsetGet);
    HHVM_ME(SimpleXMLElement, offsetSet);
    HHVM_ME(SimpleXMLElement, offsetUnset);

    Native::registerNativeDataInfo<SimpleXMLElement>(
      SimpleXMLElementLoader::className().get(), 0, Class::CallToImpl);
    Native::registerNativePropHandler<SimpleXMLElementPropHandler>(
      SimpleXMLElementLoader::className());

    /* SimpleXMLElementIterator */
    HHVM_ME(SimpleXMLElementIterator, __construct);
    HHVM_ME(SimpleXMLElementIterator, current);
    HHVM_ME(SimpleXMLElementIterator, key);
    HHVM_ME(SimpleXMLElementIterator, next);
    HHVM_ME(SimpleXMLElementIterator, rewind);
    HHVM_ME(SimpleXMLElementIterator, valid);

    Native::registerNativeDataInfo<SimpleXMLElementIterator>(
      SimpleXMLElementIteratorLoader::className().get(),
      Native::NDIFlags::NO_SWEEP
    );

    /* SimpleXMLIterator */
    HHVM_ME(SimpleXMLIterator, current);
    HHVM_ME(SimpleXMLIterator, key);
    HHVM_ME(SimpleXMLIterator, next);
    HHVM_ME(SimpleXMLIterator, rewind);
    HHVM_ME(SimpleXMLIterator, valid);
    HHVM_ME(SimpleXMLIterator, getChildren);
    HHVM_ME(SimpleXMLIterator, hasChildren);

    Native::registerNativeDataInfo<SimpleXMLIterator>(
      SimpleXMLIteratorLoader::className().get(), 0, Class::CallToImpl);
  }
} s_simplexml_extension;

///////////////////////////////////////////////////////////////////////////////
}
