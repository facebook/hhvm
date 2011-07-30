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

#ifdef TAINTED

#include <map>
#include <boost/assign.hpp>

#include <runtime/base/types.h>
#include <runtime/base/array/array_iterator.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/taint/taint_data.h>
#include <runtime/base/taint/taint_helper.h>
#include <runtime/base/taint/taint_observer.h>
#include <runtime/base/taint/taint_trace.h>

namespace HPHP {

std::map<int, std::string> taint_names = boost::assign::map_list_of
  (TAINT_BIT_HTML,    "HTML-unsafe")
  (TAINT_BIT_SQL,     "SQL-unsafe")
  (TAINT_BIT_MUTATED, "non-static");

void taint_warn_if_tainted(CStrRef s, const taint_t bit) {
  TaintData td = s.get()->getTaintDataRefConst();

  if (td.getTaint() & bit) {
    std::string buf = "Using a " + taint_names[bit] + " (tainted) string!\n";

    if (bit & TAINT_BIT_HTML) {
      buf += TaintTracer::ExtractTrace(td.getTaintTrace());
    }
    buf += "\n";

    buf += "---begin echo output---\n";
    buf += s.c_str();
    buf += "\n";
    buf += "----end echo output----\n";
    raise_warning(buf);
  }
}

void taint_array_variant(Variant& v, const std::string s, bool iskey) {
  TAINT_OBSERVER_CAP_STACK();

  if (v.isString()) {
    TaintData& td = v.asStrRef().get()->getTaintDataRef();

    std::string str;
    if (iskey) {
      str = s + " {using key}";
    } else /* isval */ {
      str = s + ": " + v.toString().c_str();
    }

    td.setTaint(TAINT_BIT_ALL | TAINT_FLAG_ORIG);
    ASSERT(!td.getTaintTrace().get());
    // We don't call TaintTracer::Trace on these strings because they:
    //   (1) are likely to be unique and not to match function calls or
    //       filenames or other params; and
    //   (2) may persist across requests, at which point they will have been
    //       cleared from the trace set and cannot be reinserted because the
    //       neither a new request nor new trace set will have been init'd.
    td.setTaintTrace(NEW(TaintTraceData)(str));
    ASSERT(!td.getTaintTrace()->getNext().get());
    ASSERT(!td.getTaintTrace()->getChild().get());
  }

  if (v.isArray()) {
    Array a = v.toArray();
    for (ArrayIter iter(a); iter; ++iter) {
      // Taint the key if it is actually a string (in cases where we have a
      // URI like /foo.php?123=hello, the key will have type int, so we skip
      // it since we only taint strings).
      Variant key = iter.first();
      std::string str = s + "[" + key.toString().c_str() + "]";
      if (key.isString()) {
        taint_array_variant(key, str, true);
      }

      // Taint the value.
      Variant value = iter.second();
      taint_array_variant(value, str, false);
    }
  }
}

}

#endif // TAINTED
