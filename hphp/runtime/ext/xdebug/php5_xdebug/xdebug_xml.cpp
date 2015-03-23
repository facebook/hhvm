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

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/zend-string.h"

#include <stdlib.h>
#include <stdio.h>

namespace HPHP {
////////////////////////////////////////////////////////////////////////////////

// Xml encode the passed string
String xdebug_xmlize(const char* string, size_t len) {
  String str(string, CopyString);
  if (len) {
    str = string_replace(str, "&","&amp;");
    str = string_replace(str, ">", "&gt;");
    str = string_replace(str, "<", "&lt;");
    str = string_replace(str, "\"", "&quot;");
    str = string_replace(str, "'", "&#39;");
    str = string_replace(str, "\n", "&#10;");
    str = string_replace(str, "\r", "&#13;");
    str = string_replace(str, "\0", "&#0;");
    return str;
  }
  return str;
}

static void xdebug_xml_return_attribute(xdebug_xml_attribute* attr,
                                        xdebug_str* output) {
  String tmp;
  xdebug_str_addl(output, " ", 1, 0);

  /* attribute name */
  tmp = xdebug_xmlize(attr->name, attr->name_len);
  xdebug_str_addl(output, tmp.get()->mutableData(), tmp.size(), 0);

  /* attribute value */
  xdebug_str_addl(output, "=\"", 2, 0);
  if (attr->value) {
    tmp = xdebug_xmlize(attr->value, attr->value_len);
    xdebug_str_add(output, tmp.get()->mutableData(), 0);
  }
  xdebug_str_addl(output, "\"", 1, 0);

  if (attr->next) {
    xdebug_xml_return_attribute(attr->next, output);
  }
}

static void xdebug_xml_return_text_node(xdebug_xml_text_node* node,
                                        xdebug_str* output) {
  xdebug_str_addl(output, "<![CDATA[", 9, 0);
  if (node->encode) {
    /* if cdata tags are in the text, then we must base64 encode */
    String encoded_str = string_base64_encode((char*) node->text,
                                              node->text_len);
    char* encoded_text = encoded_str.get()->mutableData();
    xdebug_str_add(output, encoded_text, 0);
  } else {
    xdebug_str_add(output, node->text, 0);
  }
  xdebug_str_addl(output, "]]>", 3, 0);
}

void xdebug_xml_return_node(xdebug_xml_node* node, struct xdebug_str *output) {
  xdebug_str_addl(output, "<", 1, 0);
  xdebug_str_add(output, node->tag, 0);

  if (node->text && node->text->encode) {
    xdebug_xml_add_attribute_ex(node, "encoding", "base64", 0, 0);
  }
  if (node->attribute) {
    xdebug_xml_return_attribute(node->attribute, output);
  }
  xdebug_str_addl(output, ">", 1, 0);

  if (node->child) {
    xdebug_xml_return_node(node->child, output);
  }

  if (node->text) {
    xdebug_xml_return_text_node(node->text, output);
  }

  xdebug_str_addl(output, "</", 2, 0);
  xdebug_str_add(output, node->tag, 0);
  xdebug_str_addl(output, ">", 1, 0);

  if (node->next) {
    xdebug_xml_return_node(node->next, output);
  }
}

xdebug_xml_node* xdebug_xml_node_init_ex(char* tag, int free_tag) {
  auto xml = (xdebug_xml_node*)xdmalloc(sizeof(xdebug_xml_node));

  xml->tag = tag;
  xml->text = nullptr;
  xml->child = nullptr;
  xml->attribute = nullptr;
  xml->next = nullptr;
  xml->free_tag = free_tag;

  return xml;
}

void xdebug_xml_add_attribute_exl(xdebug_xml_node* xml, char *attribute,
                                  size_t attribute_len, char *value,
                                  size_t value_len, int free_name,
                                  int free_value) {
  xdebug_xml_attribute *attr =
    (xdebug_xml_attribute*) xdmalloc(sizeof (xdebug_xml_attribute));
  xdebug_xml_attribute **ptr;

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

void xdebug_xml_add_child(xdebug_xml_node *xml, xdebug_xml_node *child) {
  xdebug_xml_node **ptr;

  ptr = &xml->child;
  while (*ptr != nullptr) {
    ptr = &((*ptr)->next);
  }
  *ptr = child;
}

static void xdebug_xml_text_node_dtor(xdebug_xml_text_node* node) {
  if (node->free_value && node->text) {
    xdfree(node->text);
  }
  xdfree(node);
}

void xdebug_xml_add_text(
  xdebug_xml_node *xml,
  const char *text,
  int free /* = 1*/
) {
  // Safe as we'll onlyr read the string from the XML node, not write/free it.
  xdebug_xml_add_text_ex(xml, const_cast<char*>(text), strlen(text), free, 0);
}

void xdebug_xml_add_text_encode(xdebug_xml_node *xml, char *text) {
  xdebug_xml_add_text_ex(xml, text, strlen(text), 1, 1);
}

void xdebug_xml_add_text_ex(xdebug_xml_node *xml, char *text, int length,
                            int free_text, int encode) {
  xdebug_xml_text_node *node =
    (xdebug_xml_text_node*) xdmalloc(sizeof(xdebug_xml_text_node));
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

static void xdebug_xml_attribute_dtor(xdebug_xml_attribute *attr) {
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
