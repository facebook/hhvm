/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2013 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.0 of the Xdebug license,    |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://xdebug.derickrethans.nl/license.php                           |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | xdebug@derickrethans.nl so we can mail you a copy immediately.       |
   +----------------------------------------------------------------------+
   | Authors:  Derick Rethans <derick@xdebug.org>                         |
   +----------------------------------------------------------------------+
 */

#include "hphp/runtime/ext/xdebug/php5_xdebug/xdebug_xml.h"

#include "hphp/runtime/ext/xdebug/php5_xdebug/xdebug_mm.h"
#include "hphp/runtime/ext/xdebug/php5_xdebug/xdebug_str.h"

#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/zend-string.h"

#include <cstdlib>
#include <cstdio>

namespace HPHP {
////////////////////////////////////////////////////////////////////////////////

namespace {
////////////////////////////////////////////////////////////////////////////////

void xdebug_xml_return_attribute(
  StringBuffer& out,
  const xdebug_xml_attribute* attr
) {
  out.append(' ');

  // Attribute name.
  out.append(xdebug_xmlize(attr->name, attr->name_len));

  // Attribute value.
  out.append("=\"");
  if (attr->value) {
    out.append(xdebug_xmlize(attr->value, attr->value_len));
  }
  out.append('"');

  if (attr->next) {
    xdebug_xml_return_attribute(out, attr->next);
  }
}

void xdebug_xml_return_text_node(
  StringBuffer& out,
  const xdebug_xml_text_node* node
) {
  out.append("<![CDATA[");
  if (node->encode) {
    // If cdata tags are in the text, then we must base64 encode.
    out.append(string_base64_encode(node->text, node->text_len));
  } else {
    out.append(node->text);
  }
  out.append("]]>");
}

void xdebug_xml_return_node_impl(StringBuffer& out, xdebug_xml_node* node) {
  out.append('<');
  out.append(node->tag);

  if (node->text && node->text->encode) {
    xdebug_xml_add_attribute_ex(node, "encoding", "base64", 0, 0);
  }
  if (node->attribute) {
    xdebug_xml_return_attribute(out, node->attribute);
  }
  out.append('>');

  if (node->child) {
    xdebug_xml_return_node_impl(out, node->child);
  }

  if (node->text) {
    xdebug_xml_return_text_node(out, node->text);
  }

  out.append("</");
  out.append(node->tag);
  out.append('>');

  if (node->next) {
    xdebug_xml_return_node_impl(out, node->next);
  }
}

void xdebug_xml_text_node_dtor(xdebug_xml_text_node* node) {
  if (node->free_value && node->text) {
    xdfree(node->text);
  }
  xdfree(node);
}


void xdebug_xml_attribute_dtor(xdebug_xml_attribute* attr) {
  if (attr->next) {
    xdebug_xml_attribute_dtor(attr->next);
  }
  if (attr->free_name) {
    xdfree(attr->name);
  }
  if (attr->free_value) {
    xdfree(attr->value);
  }
  xdfree(attr);
}

////////////////////////////////////////////////////////////////////////////////
}

/* Xml encode the passed string. */
String xdebug_xmlize(const char* string, size_t len) {
  StringBuffer out;
  for (size_t i = 0; i < len; ++i) {
    switch (string[i]) {
    case '&':  out.append("&amp;");   break;
    case '>':  out.append("&gt;");    break;
    case '<':  out.append("&lt;");    break;
    case '"':  out.append("&quot;");  break;
    case '\'': out.append("&#39;");   break;
    case '\n': out.append("&#10;");   break;
    case '\r': out.append("&#13;");   break;
    case '\0': out.append("&#0;");    break;
    default:   out.append(string[i]); break;
    }
  }
  return out.detach();
}

String xdebug_xml_return_node(xdebug_xml_node* node) {
  StringBuffer out;
  xdebug_xml_return_node_impl(out, node);
  return out.detach();
}

xdebug_xml_node* xdebug_xml_node_init(char* tag, int free_tag /* = 0 */) {
  auto xml = (xdebug_xml_node*)xdmalloc(sizeof(xdebug_xml_node));

  xml->tag = tag;
  xml->text = nullptr;
  xml->child = nullptr;
  xml->attribute = nullptr;
  xml->next = nullptr;
  xml->free_tag = free_tag;

  return xml;
}

void xdebug_xml_add_attribute_exl(xdebug_xml_node* xml, char* attribute,
                                  size_t attribute_len, char* value,
                                  size_t value_len, int free_name,
                                  int free_value) {
  auto attr = (xdebug_xml_attribute*)xdmalloc(sizeof(xdebug_xml_attribute));
  xdebug_xml_attribute** ptr;

  /* Init structure */
  attr->name = attribute;
  attr->value = value;
  attr->name_len = attribute_len;
  attr->value_len = value_len;
  attr->next = nullptr;
  attr->free_name = free_name;
  attr->free_value = free_value;

  /* Find last attribute in node */
  ptr = &xml->attribute;
  while (*ptr != nullptr) {
    ptr = &(*ptr)->next;
  }
  *ptr = attr;
}

void xdebug_xml_add_child(xdebug_xml_node* xml, xdebug_xml_node* child) {
  xdebug_xml_node** ptr;

  ptr = &xml->child;
  while (*ptr != nullptr) {
    ptr = &((*ptr)->next);
  }
  *ptr = child;
}

void xdebug_xml_add_text(
  xdebug_xml_node* xml,
  const char* text,
  int free /* = 1 */
) {
  // Safe as we'll onlyr read the string from the XML node, not write/free it.
  xdebug_xml_add_text_ex(xml, const_cast<char*>(text), strlen(text), free, 0);
}

void xdebug_xml_add_text_encode(xdebug_xml_node* xml, char* text) {
  xdebug_xml_add_text_ex(xml, text, strlen(text), 1, 1);
}

void xdebug_xml_add_text_ex(xdebug_xml_node* xml, char* text, int length,
                            int free_text, int encode) {
  auto node = (xdebug_xml_text_node*)xdmalloc(sizeof(xdebug_xml_text_node));
  node->free_value = free_text;
  node->encode = encode;

  if (xml->text) {
    xdebug_xml_text_node_dtor(xml->text);
  }
  node->text = text;
  node->text_len = length;
  xml->text = node;
  if (!encode && strstr(node->text, "]]>")) {
    node->encode = 1;
  }
}

void xdebug_xml_add_attribute(xdebug_xml_node* xml, const char* attr, int val) {
  // const-cast is okay since we are not freeing the passed attribute.
  auto const cattr = const_cast<char*>(attr);
  xdebug_xml_add_attribute_ex(xml, cattr, xdebug_sprintf("%d", val), 0, 1);
}

void xdebug_xml_add_attribute_dup(
  xdebug_xml_node* xml,
  const char* attr,
  const char* val
) {
  // const-cast is okay since we are not freeing the passed attribute.
  auto const cattr = const_cast<char*>(attr);
  xdebug_xml_add_attribute_ex(xml, cattr, xdstrdup(val), 0, 1);
}

void xdebug_xml_node_dtor(xdebug_xml_node* xml) {
  if (xml->next) {
    xdebug_xml_node_dtor(xml->next);
  }
  if (xml->child) {
    xdebug_xml_node_dtor(xml->child);
  }
  if (xml->attribute) {
    xdebug_xml_attribute_dtor(xml->attribute);
  }
  if (xml->free_tag) {
    xdfree(xml->tag);
  }
  if (xml->text) {
    xdebug_xml_text_node_dtor(xml->text);
  }
  xdfree(xml);
}

////////////////////////////////////////////////////////////////////////////////
}
