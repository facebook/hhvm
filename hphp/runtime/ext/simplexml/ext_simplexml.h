/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_SIMPLEXML_H_
#define incl_HPHP_EXT_SIMPLEXML_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/libxml/ext_libxml.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern const StaticString
  s_SimpleXMLElement,
  s_SimpleXMLElementIterator,
  s_SimpleXMLIterator;

const Class* SimpleXMLElement_classof();
const Class* SimpleXMLElementIterator_classof();
const Class* SimpleXMLIterator_classof();

bool SimpleXMLElement_propEmpty(const ObjectData* obj, const StringData* key);
Variant SimpleXMLElement_objectCast(const ObjectData* obj, int8_t type);
xmlNodePtr SimpleXMLElement_exportNode(const Object& sxe);

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_EXT_SIMPLEXML_H_
