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

#include "hphp/runtime/ext/soap/xml.h"
#include "hphp/runtime/ext/std/ext_std_file.h"
#include "hphp/runtime/ext/stream/ext_stream.h"
#include "hphp/runtime/ext/apc/ext_apc.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/file.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static bool is_blank(const xmlChar* str) {
  while (*str != '\0') {
    if (*str != ' '  && *str != 0x9 && *str != 0xa && *str != 0xd) {
      return false;
    }
    str++;
  }
  return true;
}

/* removes all empty text, comments and other insignoficant nodes */
static void cleanup_xml_node(xmlNodePtr node) {
  xmlNodePtr trav;
  xmlNodePtr del = nullptr;

  trav = node->children;
  while (trav != nullptr) {
    if (del != nullptr) {
      xmlUnlinkNode(del);
      xmlFreeNode(del);
      del = nullptr;
    }
    if (trav->type == XML_TEXT_NODE) {
      if (is_blank(trav->content)) {
        del = trav;
      }
    } else if ((trav->type != XML_ELEMENT_NODE) &&
               (trav->type != XML_CDATA_SECTION_NODE)) {
      del = trav;
    } else if (trav->children != nullptr) {
      cleanup_xml_node(trav);
    }
    trav = trav->next;
  }
  if (del != nullptr) {
    xmlUnlinkNode(del);
    xmlFreeNode(del);
  }
}

static void
soap_ignorableWhitespace(void* /*ctx*/, const xmlChar* /*ch*/, int /*len*/) {}

static void soap_Comment(void* /*ctx*/, const xmlChar* /*value*/) {}

const StaticString
  s_http("http"),
  s_timeout("timeout");

xmlDocPtr soap_xmlParseFile(const char *filename) {
  String cache_key("HPHP.SOAP.WSDL.");
  cache_key += filename;

  bool success = false;
  auto content = Variant::attach(HHVM_FN(apc_fetch)(cache_key, success));
  if (same(content, false)) {
    Variant cxt = HHVM_FN(stream_context_create)(make_dict_array(
                  s_http, make_dict_array(s_timeout, 1000)));
    auto file = File::Open(filename, "rb", 0, cast_or_null<StreamContext>(cxt));
    if (file) {
      content = HHVM_FN(stream_get_contents)(OptResource(file));
      if (!same(content, false)) {
        HHVM_FN(apc_store)(cache_key, content);
      }
    }
  }

  if (!same(content, false)) {
    String scontent = content.toString();
    xmlDocPtr ret = soap_xmlParseMemory(scontent.data(), scontent.size(),
                                        false);
    if (ret) {
      ret->URL = xmlCharStrdup(filename);
    }
    return ret;
  }
  return nullptr;
}

xmlDocPtr soap_xmlParseMemory(const void *buf, size_t buf_size,
                              bool skip_clean /*= true */) {
  xmlParserCtxtPtr ctxt = nullptr;
  xmlDocPtr ret;

/*
  xmlInitParser();
*/
  ctxt = xmlCreateMemoryParserCtxt((const char *)buf, buf_size);
  if (ctxt) {
    ctxt->keepBlanks = 0;
    ctxt->sax->ignorableWhitespace = soap_ignorableWhitespace;
    ctxt->sax->comment = soap_Comment;
    ctxt->sax->warning = nullptr;
    ctxt->sax->error = nullptr;
    /*ctxt->sax->fatalError = nullptr;*/
    auto old = HHVM_FN(libxml_disable_entity_loader)(true);
    xmlParseDocument(ctxt);
    HHVM_FN(libxml_disable_entity_loader)(old);
    if (ctxt->wellFormed) {
      ret = ctxt->myDoc;
      if (ret->URL == nullptr && ctxt->directory != nullptr) {
        ret->URL = xmlCharStrdup(ctxt->directory);
      }
    } else {
      ret = nullptr;
      xmlFreeDoc(ctxt->myDoc);
      ctxt->myDoc = nullptr;
    }
    xmlFreeParserCtxt(ctxt);
  } else {
    ret = nullptr;
  }

/*
  xmlCleanupParser();
*/

  if (!skip_clean && ret) {
    cleanup_xml_node((xmlNodePtr)ret);
  }
  return ret;
}

xmlNsPtr attr_find_ns(xmlAttrPtr node) {
  if (node->ns) {
    return node->ns;
  } else if (node->parent->ns) {
    return node->parent->ns;
  } else {
    return xmlSearchNs(node->doc, node->parent, nullptr);
  }
}

xmlNsPtr node_find_ns(xmlNodePtr node) {
  if (node->ns) {
    return node->ns;
  } else {
    return xmlSearchNs(node->doc, node, nullptr);
  }
}

bool attr_is_equal_ex(xmlAttrPtr node, char *name, char *ns) {
  if (name == nullptr || strcmp((char*)node->name, name) == 0) {
    if (ns) {
      xmlNsPtr nsPtr = attr_find_ns(node);
      if (nsPtr) {
        return (strcmp((char*)nsPtr->href, ns) == 0);
      } else {
        return false;
      }
    }
    return true;
  }
  return false;
}

bool node_is_equal_ex(xmlNodePtr node, char *name, char *ns) {
  if (name == nullptr || strcmp((char*)node->name, name) == 0) {
    if (ns) {
      xmlNsPtr nsPtr = node_find_ns(node);
      if (nsPtr) {
        return (strcmp((char*)nsPtr->href, ns) == 0);
      } else {
        return false;
      }
    }
    return true;
  }
  return false;
}


xmlAttrPtr get_attribute_ex(xmlAttrPtr node, char *name, char *ns) {
  while (node!=nullptr) {
    if (attr_is_equal_ex(node, name, ns)) {
      return node;
    }
    node = node->next;
  }
  return nullptr;
}

xmlNodePtr get_node_ex(xmlNodePtr node, char *name, char *ns) {
  while (node!=nullptr) {
    if (node_is_equal_ex(node, name, ns)) {
      return node;
    }
    node = node->next;
  }
  return nullptr;
}

xmlNodePtr get_node_recurisve_ex(xmlNodePtr node, char *name, char *ns) {
  while (node != nullptr) {
    if (node_is_equal_ex(node, name, ns)) {
      return node;
    } else if (node->children != nullptr) {
      xmlNodePtr tmp = get_node_recurisve_ex(node->children, name, ns);
      if (tmp) {
        return tmp;
      }
    }
    node = node->next;
  }
  return nullptr;
}

xmlNodePtr get_node_with_attribute_ex(xmlNodePtr node, char *name,
                                      char *name_ns, char *attribute,
                                      char *value, char *attr_ns) {
  xmlAttrPtr attr;
  while (node != nullptr) {
    if (name != nullptr) {
      node = get_node_ex(node, name, name_ns);
      if (node==nullptr) {
        return nullptr;
      }
    }

    attr = get_attribute_ex(node->properties, attribute, attr_ns);
    if (attr != nullptr && strcmp((char*)attr->children->content, value) == 0) {
      return node;
    }
    node = node->next;
  }
  return nullptr;
}

xmlNodePtr get_node_with_attribute_recursive_ex
(xmlNodePtr node, char *name, char *name_ns, char *attribute, char *value,
 char *attr_ns) {
  while (node != nullptr) {
    if (node_is_equal_ex(node, name, name_ns)) {
      xmlAttrPtr attr = get_attribute_ex(node->properties, attribute, attr_ns);
      if (attr != nullptr && !strcmp((char*)attr->children->content, value)) {
        return node;
      }
    }
    if (node->children != nullptr) {
      xmlNodePtr tmp = get_node_with_attribute_recursive_ex
        (node->children, name, name_ns, attribute, value, attr_ns);
      if (tmp) {
        return tmp;
      }
    }
    node = node->next;
  }
  return nullptr;
}

void parse_namespace(const xmlChar *inval, std::string &value,
                     std::string &ns) {
  char *found = strrchr((char*)inval, ':');

  if (found && found != (char*)inval) {
    ns = std::string((char*)inval, found - (char*)inval);
    value = ++found;
  } else {
    value = (char*)inval;
    ns.clear();
  }
}

///////////////////////////////////////////////////////////////////////////////
}
