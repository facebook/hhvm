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

#include "runtime/base/complex_types.h"
#include "util/trace.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

std::string tname(DataType t) {
  switch(t) {
#define CS(name) \
    case KindOf ## name: return std::string(#name);
    CS(Uninit)
    CS(Null)
    CS(Boolean)
    CS(Int64)
    CS(Double)
    CS(StaticString)
    CS(String)
    CS(Array)
    CS(Object)
    CS(Ref)
    CS(Class)
    CS(Any)
    CS(Uncounted)
    CS(UncountedInit)

#undef CS
    case KindOfInvalid: return std::string("Invalid");

    default: {
      char buf[128];
      sprintf(buf, "Unknown:%d", t);
      return std::string(buf);
    }
  }
}

std::string TypedValue::pretty() const  {
  char buf[20];
  sprintf(buf, "0x%lx", long(m_data.num));
  return Trace::prettyNode(tname(m_type).c_str(), std::string(buf));
}

//////////////////////////////////////////////////////////////////////

}
