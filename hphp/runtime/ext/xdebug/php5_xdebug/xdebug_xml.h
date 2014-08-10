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

#ifndef incl_XDEBUG_XML_H_
#define incl_XDEBUG_XML_H_

#include "hphp/runtime/ext/xdebug/php5_xdebug/xdebug_str.h"

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


#define xdebug_xml_node_init(t)            xdebug_xml_node_init_ex((t), 0)
#define xdebug_xml_add_attribute_ex(x,a,v,fa,fv) { \
  char *ta = (a), *tv = (v); \
  xdebug_xml_add_attribute_exl((x), (ta), strlen((ta)), (tv), \
                               strlen((tv)), fa, fv); \
}
#define xdebug_xml_add_attribute(x,a,v) \
  xdebug_xml_add_attribute_ex((x), (a), (v), 0, 0);

xdebug_xml_node *xdebug_xml_node_init_ex(char *tag, int free_tag);
void xdebug_xml_add_attribute_exl(xdebug_xml_node* xml, char *attribute,
                                  size_t attribute_len, char *value,
                                  size_t value_len, int free_name,
                                  int free_value);
void xdebug_xml_add_child(xdebug_xml_node *xml, xdebug_xml_node *child);

void xdebug_xml_add_text_ex(xdebug_xml_node *xml, char *text, int length,
                            int free_text, int encode);
void xdebug_xml_add_text(xdebug_xml_node *xml, char *text, int free = 1);
void xdebug_xml_add_text_encode(xdebug_xml_node *xml, char *text);
#define xdebug_xml_add_textl(x,t,l) \
  xdebug_xml_add_text_ex((x), (t), (l), 1, 0)
#define xdebug_xml_add_text_encodel(x,t,l) \
  xdebug_xml_add_text_ex((x), (t), (l), 1, 1)

void xdebug_xml_return_node(xdebug_xml_node* node, struct xdebug_str *output);
void xdebug_xml_node_dtor(xdebug_xml_node* xml);

#endif
