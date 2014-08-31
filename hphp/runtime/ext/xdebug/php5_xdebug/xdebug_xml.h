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
// TODO(#4489053) This could fairly easily be abstracted into a c++ class
//                This should be prioritized as this is used everywhere

#ifndef incl_XDEBUG_XML_H_
#define incl_XDEBUG_XML_H_

#include "hphp/runtime/ext/xdebug/php5_xdebug/xdebug_str.h"

namespace HPHP {
////////////////////////////////////////////////////////////////////////////////

typedef struct _xdebug_xml_attribute xdebug_xml_attribute;
typedef struct _xdebug_xml_text_node xdebug_xml_text_node;
typedef struct _xdebug_xml_node xdebug_xml_node;

struct _xdebug_xml_attribute {
  char *name;
  char *value;
  int   name_len;
  int   value_len;
  struct _xdebug_xml_attribute *next;
  int   free_name;
  int   free_value;
};

/* todo: support multiple text nodes inside an element */
struct _xdebug_xml_text_node {
  char *text;
  int   free_value;
  int   encode;
  int   text_len;
};

struct _xdebug_xml_node {
  char *tag;
  struct _xdebug_xml_text_node *text;
  struct _xdebug_xml_attribute *attribute;
  struct _xdebug_xml_node      *child;
  struct _xdebug_xml_node      *next;
  int   free_tag;
};

////////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
xdebug_xml_node* xdebug_xml_node_init_ex(char *tag, int free_tag);
inline xdebug_xml_node* xdebug_xml_node_init(char* tag) {
  return xdebug_xml_node_init_ex(tag, 0);
}

void xdebug_xml_return_node(xdebug_xml_node* node, struct xdebug_str *output);
void xdebug_xml_node_dtor(xdebug_xml_node* xml);

////////////////////////////////////////////////////////////////////////////////
// Attribute adding
void xdebug_xml_add_attribute_exl(xdebug_xml_node* xml, char *attribute,
                                  size_t attribute_len, char *value,
                                  size_t value_len, int free_name,
                                  int free_value);

inline void xdebug_xml_add_attribute_ex(xdebug_xml_node* xml,
                                        char* attr, char* val,
                                        int freeAttr, int freeVal) {
  xdebug_xml_add_attribute_exl(xml, attr, strlen(attr),
                               val, strlen(val),
                               freeAttr, freeVal);
}

// This is not a solution, just a temporary fix
inline void xdebug_xml_add_attribute(xdebug_xml_node* xml,
                                     const char* attr, const char* val) {
  // const-cast is okay since we are not freeing the passed values
  xdebug_xml_add_attribute_ex(xml,
                              const_cast<char*>(attr),
                              const_cast<char*>(val), 0, 0);
}

// Duplicates the passed values before adding the attribute. This is not a
// solution, just a temporary fix
inline void xdebug_xml_add_attribute_dup(xdebug_xml_node* xml,
                                         const char* attr, const char* val) {
  // const-cast is okay since we are not freeing the passed attribute
  xdebug_xml_add_attribute_ex(xml, const_cast<char*>(attr),
                              xdstrdup(val), 0, 1);
}

// Adds the given attribute, int value pair. This is not a solution, just a
// temporary fix
inline void xdebug_xml_add_attribute(xdebug_xml_node* xml,
                              const char* attr, int val) {
  // const-cast is okay since we are not freeing the passed attribute
  xdebug_xml_add_attribute_ex(xml, const_cast<char*>(attr),
                              xdebug_sprintf("%d", val), 0, 1);
}

////////////////////////////////////////////////////////////////////////////////
// Adding Children
void xdebug_xml_add_child(xdebug_xml_node *xml, xdebug_xml_node *child);

////////////////////////////////////////////////////////////////////////////////
// Adding text
void xdebug_xml_add_text_ex(xdebug_xml_node *xml, char *text, int length,
                            int free_text, int encode);
void xdebug_xml_add_text(xdebug_xml_node *xml, char *text, int free = 1);
void xdebug_xml_add_text_encode(xdebug_xml_node *xml, char *text);

inline void xdebug_xml_add_textl(xdebug_xml_node* xml, char* text, int length) {
  xdebug_xml_add_text_ex(xml, text, length, 1, 0);
}

inline void xdebug_xml_add_text_encodel(xdebug_xml_node* xml, char* tag,
                                        int length) {
  xdebug_xml_add_text_ex(xml, tag, length, 1, 1);
}

////////////////////////////////////////////////////////////////////////////////
}

#endif
