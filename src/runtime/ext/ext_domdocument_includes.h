/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __EXT_DOMDOCUMENT_INCLUDES_H__
#define __EXT_DOMDOCUMENT_INCLUDES_H__

#include <runtime/base/base_includes.h>

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
FORWARD_DECLARE_CLASS(domdocument);

// base class of DOMNodeList and DOMNamedNodeMap
class dom_iterable {
public:
  dom_iterable() : m_ht(NULL) {}

  sp_domdocument m_doc;
  Object m_baseobj;
  int m_nodetype;
  xmlHashTable *m_ht;
  String m_local;
  String m_ns;
  Array m_baseobjptr;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_DOMDOCUMENT_INCLUDES_H__
