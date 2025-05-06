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

#ifndef PHP_SOAP_XML_H
#define PHP_SOAP_XML_H

#include <map>
#include <string>

#include "hphp/runtime/ext/simplexml/ext_simplexml_include.h"
#include "hphp/runtime/ext/libxml/ext_libxml.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

using xmlDocMap = std::map<std::string, xmlDocPtr>;
using xmlNodeMap = std::map<std::string, xmlNodePtr>;

#define NS_STRING(ns) (ns.empty() ? nullptr : BAD_CAST(ns.c_str()))

xmlDocPtr soap_xmlParseFile(const char *filename);
xmlDocPtr soap_xmlParseMemory(const void *buf, size_t size, bool skip_clean = true);

xmlNsPtr attr_find_ns(xmlAttrPtr node);
xmlNsPtr node_find_ns(xmlNodePtr node);
bool attr_is_equal_ex(xmlAttrPtr node, const char *name, const char *ns);
bool node_is_equal_ex(xmlNodePtr node, const char *name, const char *ns);
xmlAttrPtr get_attribute_ex(xmlAttrPtr node, const char *name, const char *ns);
xmlNodePtr get_node_ex(xmlNodePtr node, const char *name, const char *ns);
xmlNodePtr get_node_recursive_ex(xmlNodePtr node, const char *name, const char *ns);
xmlNodePtr get_node_with_attribute_ex
(xmlNodePtr node, const char *name, const char *name_ns, const char *attribute, const char *value,
 const char *attr_ns);
xmlNodePtr get_node_with_attribute_recursive_ex
(xmlNodePtr node, const char *name, const char *name_ns, const char *attribute, const char *value,
 const char *attr_ns);
void parse_namespace(const xmlChar *inval, std::string &value,
                     std::string &ns);

inline xmlAttrPtr get_attribute(xmlAttrPtr node, const char* name) {
  return get_attribute_ex(node, name, nullptr);
}
inline xmlNodePtr get_node(xmlNodePtr node, const char* name) {
  return get_node_ex(node, name, nullptr);
}
inline xmlNodePtr get_node_recursive(xmlNodePtr node, const char* name) {
  return get_node_recursive_ex(node, name, nullptr);
}
inline xmlNodePtr get_node_with_attribute(xmlNodePtr node,
                                          const char* name,
                                          const char* attr,
                                          const char* val) {
  return get_node_with_attribute_ex(node, name, nullptr, attr, val, nullptr);
}
inline xmlNodePtr get_node_with_attribute_recursive(xmlNodePtr node,
                                                    const char* name,
                                                    const char* attr,
                                                    const char* val) {
  return get_node_with_attribute_recursive_ex(node, name, nullptr,
                                              attr, val, nullptr);
}
inline bool node_is_equal(xmlNodePtr node, const char* name) {
  return node_is_equal_ex(node, name, nullptr);
}
inline bool attr_is_equal(xmlAttrPtr node, const char* name) {
  return attr_is_equal_ex(node, name, nullptr);
}

#define FOREACHATTRNODE(n,c,i)      FOREACHATTRNODEEX(n,c,nullptr,i)
#define FOREACHATTRNODEEX(n,c,ns,i)             \
  do {                                          \
    if (n == nullptr) {                         \
      break;                                    \
    }                                           \
    if (c) {                                    \
      i = get_attribute_ex(n,c,ns);             \
    } else {                                    \
      i = n;                                    \
    }                                           \
    if (i != nullptr) {                         \
      n = i;

#define FOREACHNODE(n,c,i)      FOREACHNODEEX(n,c,nullptr,i)
#define FOREACHNODEEX(n,c,ns,i)                 \
  do {                                          \
    if (n == nullptr) {                         \
      break;                                    \
    }                                           \
    if (c != nullptr) {                         \
      i = get_node_ex(n,c,nullptr);             \
    } else {                                    \
      i = n;                                    \
    }                                           \
    if (i != nullptr) {                         \
      n = i;

#define ENDFOREACH(n)                           \
  }                                             \
} while ((n = n->next));

///////////////////////////////////////////////////////////////////////////////
}

#endif
