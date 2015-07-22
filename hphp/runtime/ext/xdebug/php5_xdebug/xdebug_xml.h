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
// TODO(#3704) This could fairly easily be abstracted into a c++ class

#ifndef incl_XDEBUG_XML_H_
#define incl_XDEBUG_XML_H_

#include <cstring>

namespace HPHP {
////////////////////////////////////////////////////////////////////////////////

struct String;

struct xdebug_xml_attribute {
  char* name;
  char* value;
  int name_len;
  int value_len;
  xdebug_xml_attribute* next;
  int free_name;
  int free_value;
};

/* todo: support multiple text nodes inside an element */
struct xdebug_xml_text_node {
  char* text;
  int free_value;
  int encode;
  int text_len;
};

struct xdebug_xml_node {
  char* tag;
  xdebug_xml_text_node* text;
  xdebug_xml_attribute* attribute;
  xdebug_xml_node* child;
  xdebug_xml_node* next;
  int free_tag;
};

////////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
xdebug_xml_node* xdebug_xml_node_init(char* tag, int free_tag = 0);
void xdebug_xml_node_dtor(xdebug_xml_node*);

String xdebug_xml_return_node(xdebug_xml_node*);

////////////////////////////////////////////////////////////////////////////////
// Attribute adding
void xdebug_xml_add_attribute_exl(xdebug_xml_node* xml, char *attribute,
                                  size_t attribute_len, char *value,
                                  size_t value_len, int free_name,
                                  int free_value);

inline void xdebug_xml_add_attribute_ex(xdebug_xml_node* xml,
                                        const char* attr, const char* val,
                                        int freeAttr, int freeVal) {
  // const_cast is safe since we are not freeing the strings or writing into
  // them.
  xdebug_xml_add_attribute_exl(
    xml,
    const_cast<char*>(attr),
    strlen(attr),
    const_cast<char*>(val),
    strlen(val),
    freeAttr,
    freeVal
  );
}

// This is not a solution, just a temporary fix
inline void xdebug_xml_add_attribute(xdebug_xml_node* xml,
                                     const char* attr, const char* val) {
  xdebug_xml_add_attribute_ex(xml, attr, val, 0, 0);
}

/* Duplicates the passed value before adding the attribute. */
void xdebug_xml_add_attribute_dup(xdebug_xml_node*, const char*, const char*);

/* Adds the given attribute, int value pair. */
void xdebug_xml_add_attribute(xdebug_xml_node* xml, const char* attr, int val);

////////////////////////////////////////////////////////////////////////////////
// Adding Children
void xdebug_xml_add_child(xdebug_xml_node *xml, xdebug_xml_node *child);

////////////////////////////////////////////////////////////////////////////////
// Adding text
void xdebug_xml_add_text_ex(xdebug_xml_node *xml, char *text, int length,
                            int free_text, int encode);
void xdebug_xml_add_text(xdebug_xml_node *xml, const char* text, int free = 1);
void xdebug_xml_add_text_encode(xdebug_xml_node *xml, char *text);

inline void xdebug_xml_add_textl(xdebug_xml_node* xml, char* text, int length) {
  xdebug_xml_add_text_ex(xml, text, length, 1, 0);
}

inline void xdebug_xml_add_text_encodel(xdebug_xml_node* xml, char* tag,
                                        int length) {
  xdebug_xml_add_text_ex(xml, tag, length, 1, 1);
}

String xdebug_xmlize(const char*, size_t);

////////////////////////////////////////////////////////////////////////////////
}

#endif
