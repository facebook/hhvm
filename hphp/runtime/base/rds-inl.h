/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_RUNTIME_BASE_RDS_INL_H_
#define incl_HPHP_RUNTIME_BASE_RDS_INL_H_

namespace HPHP { namespace RDS {

//////////////////////////////////////////////////////////////////////

inline Header* header() {
  return static_cast<Header*>(tl_base);
}

//////////////////////////////////////////////////////////////////////

extern __thread std::aligned_storage<sizeof(Array),alignof(Array)>::type
  s_constantsStorage;

ALWAYS_INLINE Array& s_constants() {
  void* vp = &s_constantsStorage;
  return *static_cast<Array*>(vp);
}

//////////////////////////////////////////////////////////////////////

}}

#endif
