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

#ifndef incl_HPHP_EXT_DOMDOCUMENT_INCLUDES_H_
#define incl_HPHP_EXT_DOMDOCUMENT_INCLUDES_H_

#include "hphp/runtime/base/base-includes.h"

///////////////////////////////////////////////////////////////////////////////

#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#include <libxml/relaxng.h>
#include <libxml/tree.h>
#include <libxml/uri.h>
#include <libxml/xmlerror.h>
#include <libxml/xmlschemas.h>
#include <libxml/xmlwriter.h>
#include <libxml/xinclude.h>
#include <libxml/hash.h>
#include <libxml/c14n.h>
#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/xpointer.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// so every dom node holds a document object to avoid its early deletion
FORWARD_DECLARE_CLASS(DOMDocument);

typedef hphp_hash_set<xmlNodePtr, pointer_hash<xmlNode> > XmlNodeSet;

// base class of DOMNodeList and DOMNamedNodeMap
class dom_iterable {
public:
  dom_iterable() : m_ht(NULL) {}

  p_DOMDocument m_doc;
  Object m_baseobj;
  int m_nodetype;
  xmlHashTable *m_ht;
  String m_local;
  String m_ns;
  Array m_baseobjptr;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_DOMDOCUMENT_INCLUDES_H_
