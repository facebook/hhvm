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

#ifndef __HPHP_TAINT_TRACE_H__
#define __HPHP_TAINT_TRACE_H__

#ifdef TAINTED

#include <runtime/base/complex_types.h>
#include <runtime/base/util/countable.h>
#include <runtime/base/util/request_local.h>
#include <runtime/base/util/smart_ptr.h>
#include <runtime/base/memory/smart_allocator.h>
#include <runtime/base/taint/taint_trace_node.h>

namespace HPHP {

/*
 * Representation of whatever data we consider a trace, in the form of a ref-
 * counted linked list of Strings. We naively consider the String(s) to form
 * a backtrace if there are more than one, and an original string otherwise.
 */
class TaintTraceData : public Countable {
public:
  TaintTraceData() { }

  TaintTraceData(String str) : m_string(str) { }

  const TaintTraceDataPtr& getNext() const { return m_next; }
  String getStr() const { return m_string; }

  TaintTraceDataPtr attachData(String str) {
    m_next = NEW(TaintTraceData)(str);
    return m_next;
  }

  // SmartAllocator methods
  DECLARE_SMART_ALLOCATION_NOCALLBACKS(TaintTraceData);
  void dump() const { }

private:
  TaintTraceDataPtr m_next;
  String m_string;
};

/*
 * A wrapper around StringMap (hash_set<String>) for RequestLocal use.
 */
class TaintTraceStore : public RequestEventHandler {
public:
  virtual void requestInit() { }
  virtual void requestShutdown() { m_stringset.clear(); }

  const String insert(String str) {
    return *(m_stringset.insert(str).first);
  }
  bool find(String str) {
    return (m_stringset.find(str) != m_stringset.end());
  }

private:
  StringSet m_stringset;
};

/*
 * Public access class for the RequestLocal trace data store. Does all
 * nonlocal work for tracing.
 */
class TaintTracer {
public:
  static String Trace(String str, bool copy=false);

  static TaintTraceDataPtr CreateTrace();
  static std::string ExtractTrace(const TaintTraceNodePtr& root);

  static int GetDepth() { return s_btdepth; }
  static void SetDepth(int n) { s_btdepth = n; }

private:
  static String TraceFrameAsString(Array frame, int i);
  static Array TraceFrameAsArray(Array frame);

  static void ExtractInternal(const TaintTraceNodePtr& root,
      hphp_string_set& sourceset, hphp_string_set& frameset);

  DECLARE_STATIC_REQUEST_LOCAL(TaintTraceStore, s_tracestore);
  static int s_btdepth;
};

}

#endif // TAINTED

#endif // __HPHP_TAINT_TRACE_H__
