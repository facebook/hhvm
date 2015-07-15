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

#include "hphp/runtime/ext/simplexml/ext_simplexml.h"
#include <vector>
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/ext/domdocument/ext_domdocument.h"
#include "hphp/runtime/ext/std/ext_std_file.h"
#include "hphp/runtime/ext/libxml/ext_libxml.h"
#include "hphp/system/systemlib.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {

IMPLEMENT_DEFAULT_EXTENSION_VERSION(SimpleXML, 0.1);

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

static void sxe_add_registered_namespaces(c_SimpleXMLElement* sxe,
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

static void sxe_add_namespaces(c_SimpleXMLElement* sxe, xmlNodePtr node,
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

static Object _node_as_zval(c_SimpleXMLElement* sxe, xmlNodePtr node,
                            SXE_ITER itertype, const char* name,
                            const xmlChar* nsprefix, bool isprefix) {
  Object obj = create_object(sxe->getClassName(), Array(), false);
  auto subnode = cast<c_SimpleXMLElement>(obj);
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

static inline bool match_ns(c_SimpleXMLElement* sxe, xmlNodePtr node,
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

static xmlNodePtr sxe_get_element_by_offset(c_SimpleXMLElement* sxe,
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

static xmlNodePtr php_sxe_iterator_fetch(c_SimpleXMLElement* sxe,
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

static void php_sxe_move_forward_iterator(c_SimpleXMLElement* sxe) {
  xmlNodePtr node = nullptr;
  if (!sxe->iter.data.isNull()) {
    auto intern = cast<c_SimpleXMLElement>(sxe->iter.data);
    node = intern->nodep();
    sxe->iter.data = nullptr;
  }

  if (node) {
    php_sxe_iterator_fetch(sxe, node->next, 1);
  }
}

static xmlNodePtr php_sxe_reset_iterator(c_SimpleXMLElement* sxe,
                                         bool use_data) {
  if (!sxe->iter.data.isNull()) {
    sxe->iter.data = nullptr;
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

static int64_t php_sxe_count_elements_helper(c_SimpleXMLElement* sxe) {
  Object data = sxe->iter.data;
  sxe->iter.data = nullptr;

  xmlNodePtr node = php_sxe_reset_iterator(sxe, false);
  int64_t count = 0;
  while (node) {
    count++;
    node = php_sxe_iterator_fetch(sxe, node->next, 0);
  }

  sxe->iter.data = data;
  return count;
}

static xmlNodePtr php_sxe_get_first_node(c_SimpleXMLElement* sxe,
                                         xmlNodePtr node) {
  if (sxe && sxe->iter.type != SXE_ITER_NONE) {
    php_sxe_reset_iterator(sxe, true);
    xmlNodePtr retnode = nullptr;
    if (!sxe->iter.data.isNull()) {
      retnode = cast<c_SimpleXMLElement>(sxe->iter.data)->nodep();
    }
    return retnode;
  } else {
    return node;
  }
}

xmlNodePtr simplexml_export_node(c_SimpleXMLElement* sxe) {
  return php_sxe_get_first_node(sxe, sxe->nodep());
}

static Variant cast_object(char* contents, int type) {
  String str = String((char*)contents);
  Variant obj;
  switch (type) {
    case HPHP::KindOfString:
      obj = str;
      break;
    case HPHP::KindOfInt64:
      obj = toInt64(str);
      break;
    case HPHP::KindOfDouble:
      obj = toDouble(str);
      break;
  }
  return obj;
}

static Object sxe_prop_dim_read(c_SimpleXMLElement* sxe, const Variant& member,
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
      return nullptr;
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
      return nullptr;
    }
  }

  Object return_value = nullptr;
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

static void sxe_prop_dim_delete(c_SimpleXMLElement* sxe, const Variant& member,
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

static bool sxe_prop_dim_exists(c_SimpleXMLElement* sxe, const Variant& member,
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

static Variant _get_base_node_value(c_SimpleXMLElement* sxe_ref,
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
    Object obj = create_object(sxe_ref->getClassName(), Array(), false);
    auto subnode = cast<c_SimpleXMLElement>(obj);
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
      Array arr = Array::Create();
      arr.append(existVal);
      arr.append(value);
      rv.set(sName, arr);
    }
  } else {
    rv.set(sName, value);
  }
}

static void sxe_get_prop_hash(c_SimpleXMLElement* sxe, bool is_debug,
                              Array& rv, bool isBoolCast = false) {
  rv.clear();

  Object iter_data = nullptr;
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
      Array zattr = Array::Create();
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
        rv.set(String("@attributes"), zattr);
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
      if (!node->children || !node->parent || node->children->next ||
          node->children->children ||
          node->parent->children == node->parent->last) {
        node = node->children;
      } else {
        iter_data = sxe->iter.data;
        sxe->iter.data = nullptr;

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

static Variant sxe_object_cast(c_SimpleXMLElement* sxe, int8_t type) {
  if (type == HPHP::KindOfBoolean) {
    xmlNodePtr node = php_sxe_get_first_node(sxe, nullptr);
    if (node) return true;
    Array properties = Array::Create();
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

  Variant obj = cast_object((char*)contents, type);

  if (contents) {
    xmlFree(contents);
  }
  return obj;
}

static bool sxe_prop_dim_write(c_SimpleXMLElement* sxe, const Variant& member,
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

static Class* class_from_name(const String& class_name, const char* callee) {
  Class* cls;
  if (!class_name.empty()) {
    cls = Unit::loadClass(class_name.get());
    if (!cls) {
      throw_invalid_argument("class not found: %s", class_name.data());
      return nullptr;
    }
    if (!cls->classof(c_SimpleXMLElement::classof())) {
      throw_invalid_argument(
        "%s() expects parameter 2 to be a class name "
        "derived from SimpleXMLElement, '%s' given",
        callee,
        class_name.data());
      return nullptr;
    }
  } else {
    cls = c_SimpleXMLElement::classof();
  }
  return cls;
}

Variant f_simplexml_import_dom(
  const Object& node,
  const String& class_name /* = "SimpleXMLElement" */) {
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
    Class* cls = class_from_name(class_name, "simplexml_import_dom");
    if (!cls) {
      return init_null();
    }
    Object obj = create_object(cls->nameStr(), Array(), false);
    auto sxe = cast<c_SimpleXMLElement>(obj);
    sxe->node = libxml_register_node(nodep);
    return obj;
  } else {
    raise_warning("Invalid Nodetype to import");
    return init_null();
  }
  return false;
}

Variant f_simplexml_load_string(
  const String& data,
  const String& class_name /* = "SimpleXMLElement" */,
  int64_t options /* = 0 */,
  const String& ns /* = "" */,
  bool is_prefix /* = false */) {
  SYNC_VM_REGS_SCOPED();
  Class* cls = class_from_name(class_name, "simplexml_load_string");
  if (!cls) {
    return init_null();
  }

  xmlDocPtr doc = xmlReadMemory(data.data(), data.size(), nullptr,
                                     nullptr, options);
  if (!doc) {
    return false;
  }

  Object obj = create_object(cls->nameStr(), Array(), false);
  auto sxe = cast<c_SimpleXMLElement>(obj);
  sxe->node = libxml_register_node(xmlDocGetRootElement(doc));
  sxe->iter.nsprefix = ns.size() ? xmlStrdup((xmlChar*)ns.data()) : nullptr;
  sxe->iter.isprefix = is_prefix;
  return obj;
}

Variant f_simplexml_load_file(const String& filename,
                              const String& class_name /* = "SimpleXMLElement" */,
                              int64_t options /* = 0 */, const String& ns /* = "" */,
                              bool is_prefix /* = false */) {
  SYNC_VM_REGS_SCOPED();
  Class* cls = class_from_name(class_name, "simplexml_load_file");
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
  auto sxe = cast<c_SimpleXMLElement>(obj);
  sxe->node = libxml_register_node(xmlDocGetRootElement(doc));
  sxe->iter.nsprefix = ns.size() ? xmlStrdup((xmlChar*)ns.data()) : nullptr;
  sxe->iter.isprefix = is_prefix;
  return obj;
}

///////////////////////////////////////////////////////////////////////////////
// SimpleXMLElement

c_SimpleXMLElement::c_SimpleXMLElement(Class* cb) :
  SimpleXMLElementBase(cb), node(nullptr), xpath(nullptr)
{
  iter.name     = nullptr;
  iter.nsprefix = nullptr;
  iter.isprefix = false;
  iter.type     = SXE_ITER_NONE;
  iter.data     = nullptr;
}

c_SimpleXMLElement::~c_SimpleXMLElement() {
  c_SimpleXMLElement::sweep();
}

void c_SimpleXMLElement::sweep() {
  if (iter.name) {
    xmlFree(iter.name);
    iter.name = nullptr;
  }
  if (iter.nsprefix) {
    xmlFree(iter.nsprefix);
    iter.nsprefix = nullptr;
  }
  if (xpath) {
    xmlXPathFreeContext(xpath);
    xpath = nullptr;
  }
}

void c_SimpleXMLElement::t___construct(const String& data,
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
  iter.nsprefix = !ns.empty() ? xmlStrdup((xmlChar*)ns.data()) : nullptr;
  iter.isprefix = is_prefix;
  node = libxml_register_node(xmlDocGetRootElement(docp));
}

Variant c_SimpleXMLElement::t_xpath(const String& path) {
  if (iter.type == SXE_ITER_ATTRLIST) {
    return init_null(); // attributes don't have attributes
  }

  if (!xpath) {
    xpath = xmlXPathNewContext(docp());
  }
  if (!nodep()) {
    node = libxml_register_node(xmlDocGetRootElement(docp()));
  }

  xmlNodePtr nodeptr = php_sxe_get_first_node(this, this->nodep());
  xpath->node = nodeptr;

  xmlNsPtr* ns = xmlGetNsList(docp(), nodeptr);
  int64_t nsnbr = 0;
  if (ns != nullptr) {
    while (ns[nsnbr] != nullptr) {
      nsnbr++;
    }
  }

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

  Array ret = Array::Create();
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
          obj = _node_as_zval(this, nodeptr->parent, SXE_ITER_NONE, nullptr,
                              nullptr, false);
        } else if (nodeptr->type == XML_ATTRIBUTE_NODE) {
          obj = _node_as_zval(this, nodeptr->parent, SXE_ITER_ATTRLIST,
                              (char*)nodeptr->name, nodeptr->ns ?
                                (xmlChar*)nodeptr->ns->href : nullptr, false);
        } else {
          obj = _node_as_zval(this, nodeptr, SXE_ITER_NONE, nullptr, nullptr,
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

bool c_SimpleXMLElement::t_registerxpathnamespace(const String& prefix,
                                                  const String& ns) {
  if (!xpath) {
    xpath = xmlXPathNewContext(docp());
  }

  if (xmlXPathRegisterNs(xpath,
                         (xmlChar*)prefix.data(),
                         (xmlChar*)ns.data()) != 0) {
    return false;
  }
  return true;
}

Variant c_SimpleXMLElement::t_savexml(const String& filename /* = "" */) {
  return t_asxml(filename);
}

Variant c_SimpleXMLElement::t_asxml(const String& filename /* = "" */) {
  xmlNodePtr node = this->nodep();
  xmlOutputBufferPtr outbuf = nullptr;

  if (filename.size()) {
    node = php_sxe_get_first_node(this, node);

    if (node) {
      xmlDocPtr doc = docp();
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

  node = php_sxe_get_first_node(this, node);

  if (node) {
    xmlDocPtr doc = docp();
    if (node->parent && (XML_DOCUMENT_NODE == node->parent->type)) {
      xmlChar* strval;
      int strval_len;
      xmlDocDumpMemoryEnc(doc, &strval, &strval_len,
                          (const char*)doc->encoding);
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
      String ret = String(str);
      xmlOutputBufferClose(outbuf);
      return ret;
    }
  } else {
    return false;
  }
  return false;
}

Array c_SimpleXMLElement::t_getnamespaces(bool recursive /* = false */) {
  Array ret = Array::Create();
  xmlNodePtr node = this->nodep();
  node = php_sxe_get_first_node(this, node);
  if (node) {
    if (node->type == XML_ELEMENT_NODE) {
      sxe_add_namespaces(this, node, recursive, ret);
    } else if (node->type == XML_ATTRIBUTE_NODE && node->ns) {
      sxe_add_namespace_name(ret, node->ns);
    }
  }
  return ret;
}

Array c_SimpleXMLElement::t_getdocnamespaces(bool recursive /* = false */,
                                             bool from_root /* = true */) {
  xmlNodePtr node =
    from_root ? xmlDocGetRootElement(docp())
              : this->nodep();
  Array ret = Array::Create();
  sxe_add_registered_namespaces(this, node, recursive, ret);
  return ret;
}

Object c_SimpleXMLElement::t_children(const String& ns /* = "" */,
                                      bool is_prefix /* = false */) {
  if (iter.type == SXE_ITER_ATTRLIST) {
    return Object(); /* attributes don't have attributes */
  }

  xmlNodePtr node = this->nodep();
  node = php_sxe_get_first_node(this, node);
  return _node_as_zval(this, node, SXE_ITER_CHILD, nullptr,
                       (xmlChar*)ns.data(), is_prefix);
}

String c_SimpleXMLElement::t_getname() {
  xmlNodePtr node = this->nodep();
  node = php_sxe_get_first_node(this, node);
  if (node) {
    return String((char*)node->name);
  }
  return empty_string();
}

Object c_SimpleXMLElement::t_attributes(const String& ns /* = "" */,
                                        bool is_prefix /* = false */) {
  if (iter.type == SXE_ITER_ATTRLIST) {
    return Object(); /* attributes don't have attributes */
  }

  xmlNodePtr node = this->nodep();
  node = php_sxe_get_first_node(this, node);
  return _node_as_zval(this, node, SXE_ITER_ATTRLIST, nullptr,
                       (xmlChar*)ns.data(), is_prefix);
}

Variant c_SimpleXMLElement::t_addchild(const String& qname,
                                       const String& value /* = null_string */,
                                       const Variant& ns /* = null */) {
  if (qname.empty()) {
    raise_warning("Element name is required");
    return init_null();
  }

  xmlNodePtr node = this->nodep();

  if (iter.type == SXE_ITER_ATTRLIST) {
    raise_warning("Cannot add element to attributes");
    return init_null();
  }

  node = php_sxe_get_first_node(this, node);

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

  Object ret = _node_as_zval(this, newnode, SXE_ITER_NONE, (char*)localname,
                             prefix, false);

  xmlFree(localname);
  if (prefix != nullptr) {
    xmlFree(prefix);
  }
  return ret;
}

void c_SimpleXMLElement::t_addattribute(const String& qname,
                                        const String& value /* = null_string */,
                                        const String& ns /* = null_string */) {
  if (qname.size() == 0) {
    raise_warning("Attribute name is required");
    return;
  }

  xmlNodePtr node = this->nodep();
  node = php_sxe_get_first_node(this, node);

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

String c_SimpleXMLElement::t___tostring() {
  return sxe_object_cast(this, HPHP::KindOfString);
}

Variant c_SimpleXMLElement::t___get(Variant name) {
  return sxe_prop_dim_read(this, name, true, false);
}

Variant c_SimpleXMLElement::t___unset(Variant name) {
  sxe_prop_dim_delete(this, name, true, false);
  return init_null();
}

bool c_SimpleXMLElement::t___isset(Variant name) {
  return sxe_prop_dim_exists(this, name, false, true, false);
}

Variant c_SimpleXMLElement::t___set(Variant name, Variant value) {
  return sxe_prop_dim_write(this, name, value, true, false, nullptr);
}

bool c_SimpleXMLElement::PropEmpty(ObjectData* obj, const StringData* key) {
  return !sxe_prop_dim_exists(static_cast<c_SimpleXMLElement*>(obj),
    Variant(key->toCppString()), true, true, false);
}

c_SimpleXMLElement* c_SimpleXMLElement::Clone(ObjectData* obj) {
  auto sxe = static_cast<c_SimpleXMLElement*>(obj);
  c_SimpleXMLElement *clone =
    static_cast<c_SimpleXMLElement*>(obj->cloneImpl());

  clone->iter.isprefix = sxe->iter.isprefix;
  if (sxe->iter.name != nullptr) {
    clone->iter.name = xmlStrdup((xmlChar*)sxe->iter.name);
  }
  if (sxe->iter.nsprefix != nullptr) {
    clone->iter.nsprefix = xmlStrdup((xmlChar*)sxe->iter.nsprefix);
  }
  clone->iter.type = sxe->iter.type;

  if (sxe->nodep()) {
    clone->node =
      libxml_register_node(xmlDocCopyNode(sxe->nodep(), sxe->docp(), 1));
  }

  return clone;
}

bool c_SimpleXMLElement::ToBool(const ObjectData* obj) noexcept {
  return sxe_object_cast(const_cast<c_SimpleXMLElement*>(
    static_cast<const c_SimpleXMLElement*>(obj)),
    HPHP::KindOfBoolean).toBoolean();
}

int64_t c_SimpleXMLElement::ToInt64(const ObjectData* obj) noexcept {
  return sxe_object_cast(const_cast<c_SimpleXMLElement*>(
    static_cast<const c_SimpleXMLElement*>(obj)), HPHP::KindOfInt64).toInt64();
}

double c_SimpleXMLElement::ToDouble(const ObjectData* obj) noexcept {
  return sxe_object_cast(const_cast<c_SimpleXMLElement*>(
    static_cast<const c_SimpleXMLElement*>(obj)),
    HPHP::KindOfDouble).toDouble();
}

Array c_SimpleXMLElement::ToArray(const ObjectData* obj) {
  c_SimpleXMLElement *sxe = const_cast<c_SimpleXMLElement*>(
      static_cast<const c_SimpleXMLElement*>(obj));
  Array properties = Array::Create();
  sxe_get_prop_hash(sxe, true, properties);
  return properties;
}

Variant c_SimpleXMLElement::t_getiterator() {
  Object obj = create_object(c_SimpleXMLElementIterator::classof()->nameStr(),
                             Array(), false);
  auto iter = cast<c_SimpleXMLElementIterator>(obj);
  iter->sxe = this;
  iter->sxe->incRefCount();
  return obj;
}

int64_t c_SimpleXMLElement::t_count() {
  return php_sxe_count_elements_helper(this);
}

///////////////////////////////////////////////////////////////////////////////
// ArrayAccess

bool c_SimpleXMLElement::t_offsetexists(const Variant& index) {
  return sxe_prop_dim_exists(this, index, false, false, true);
}

Variant c_SimpleXMLElement::t_offsetget(const Variant& index) {
  return sxe_prop_dim_read(this, index, false, true);
}

void c_SimpleXMLElement::t_offsetset(const Variant& index, const Variant& newvalue) {
  sxe_prop_dim_write(this, index, newvalue, false, true, nullptr);
}

void c_SimpleXMLElement::t_offsetunset(const Variant& index) {
  sxe_prop_dim_delete(this, index, false, true);
}

///////////////////////////////////////////////////////////////////////////////
// Iterator

c_SimpleXMLElementIterator::c_SimpleXMLElementIterator(Class* cb) :
    ExtObjectData(cb), sxe(nullptr) {
}

c_SimpleXMLElementIterator::~c_SimpleXMLElementIterator() { }

void c_SimpleXMLElementIterator::t___construct() {
}

Variant c_SimpleXMLElementIterator::t_current() {
  return sxe->iter.data;
}

Variant c_SimpleXMLElementIterator::t_key() {
  Object curobj = sxe->iter.data;
  xmlNodePtr curnode = curobj.isNull()
    ? nullptr
    : cast<c_SimpleXMLElement>(curobj)->nodep();
  if (curnode) {
    return String((char*)curnode->name);
  } else {
    return init_null();
  }
}

Variant c_SimpleXMLElementIterator::t_next() {
  php_sxe_move_forward_iterator(sxe.get());
  return init_null();
}

Variant c_SimpleXMLElementIterator::t_rewind() {
  php_sxe_reset_iterator(sxe.get(), true);
  return init_null();
}

Variant c_SimpleXMLElementIterator::t_valid() {
  return !sxe->iter.data.isNull();
}

///////////////////////////////////////////////////////////////////////////////
// SimpleXMLIterator

Variant c_SimpleXMLIterator::t_current() {
  return iter.data;
}

Variant c_SimpleXMLIterator::t_key() {
  Object curobj = iter.data;
  xmlNodePtr curnode = curobj.isNull()
    ? nullptr
    : cast<c_SimpleXMLElement>(curobj)->nodep();
  if (curnode) {
    return String((char*)curnode->name);
  } else {
    return init_null();
  }
}

Variant c_SimpleXMLIterator::t_next() {
  php_sxe_move_forward_iterator(this);
  return init_null();
}

Variant c_SimpleXMLIterator::t_rewind() {
  php_sxe_reset_iterator(this, true);
  return init_null();
}

Variant c_SimpleXMLIterator::t_valid() {
  return !iter.data.isNull();
}

bool c_SimpleXMLIterator::t_haschildren() {
  if(iter.data.isNull()) {
    return false;
  }
  return cast<c_SimpleXMLElement>(t_getchildren())->t_count() != 0;
}

Object c_SimpleXMLIterator::t_getchildren() {
  if(iter.data.isNull()) {
    return null_object;
  }
  return cast<c_SimpleXMLElement>(iter.data)->t_children();
}

///////////////////////////////////////////////////////////////////////////////
}
