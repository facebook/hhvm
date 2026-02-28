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

#pragma once

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/libxml/ext_libxml.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct SimpleXMLElementLoader : SystemLib::ClassLoader<"SimpleXMLElement"> {};
struct SimpleXMLElementIteratorLoader :
  SystemLib::ClassLoader<"SimpleXMLElementIterator"> {};
struct SimpleXMLIteratorLoader : SystemLib::ClassLoader<"SimpleXMLIterator"> {};

Array SimpleXMLElement_darrayCast(const ObjectData* obj);
Variant SimpleXMLElement_objectCast(const ObjectData* obj, DataType type);
xmlNodePtr SimpleXMLElement_exportNode(const Object& sxe);

///////////////////////////////////////////////////////////////////////////////
}
