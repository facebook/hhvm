/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include "hphp/runtime/base/complex_types.h"
#include "hphp/runtime/base/variable_serializer.h"

namespace HPHP {

IMPLEMENT_SMART_ALLOCATION(RefData);

RefData::~RefData() {
  assert(m_magic == kMagic);
  tvAsVariant(&m_tv).~Variant();
}

void RefData::dump() const {
  VariableSerializer vs(VariableSerializer::VarDump);
  String ret(vs.serialize(tvAsCVarRef(&m_tv), true));
  printf("RefData: %s", ret.c_str());
}

}
