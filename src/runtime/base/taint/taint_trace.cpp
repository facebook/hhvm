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

#include <runtime/base/frame_injection.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/util/extended_logger.h>
#include <runtime/base/array/array_iterator.h>
#include <runtime/base/taint/taint_observer.h>
#include <runtime/base/taint/taint_trace.h>

namespace HPHP {

IMPLEMENT_REQUEST_LOCAL(TaintTracerRequestData, TaintTracer::s_requestdata);
IMPLEMENT_SMART_ALLOCATION(TaintTraceData);
IMPLEMENT_SMART_ALLOCATION(TaintTraceNode);

/*
 * TaintTraceDataPtr methods
 */
TaintTraceDataPtr::TaintTraceDataPtr() { }

TaintTraceDataPtr::~TaintTraceDataPtr() { }

TaintTraceDataPtr::TaintTraceDataPtr(TaintTraceData* ttd)
  : SmartPtr<TaintTraceData>(ttd) { }

TaintTraceDataPtr::TaintTraceDataPtr(const TaintTraceDataPtr& ttd)
  : SmartPtr<TaintTraceData>(ttd) { }

/*
 * TaintTracer methods
 */
String TaintTracer::Trace(String str, bool copy, bool truncate) {
  assert(!TaintObserver::IsActive());

  if (copy && !s_requestdata->find(str)) {
    int len = (truncate && RuntimeOption::TaintTraceMaxStrlen < str.size()) ?
      RuntimeOption::TaintTraceMaxStrlen : str.size();
    String str_copy(str.c_str(), len, CopyString);
    return s_requestdata->insert(str_copy);
  } else {
    return s_requestdata->insert(str);
  }
}

String TaintTracer::TraceFrameAsString(Array frame, int i) {
  return Trace(ExtendedLogger::StringOfFrame(frame, i));
}

Array TaintTracer::TraceFrameAsArray(Array frame) {
  if (frame.exists("function")) {
    frame.set("function", Trace(frame["function"].toString()));
  }
  if (frame.exists("class")) {
    frame.set("class", Trace(frame["class"].toString()));
  }
  if (frame.exists("type")) {
    frame.set("type", Trace(frame["type"].toString()));
  }
  if (frame.exists("file")) {
    frame.set("file", Trace(frame["file"].toString()));
  }

  return frame;
}

TaintTraceDataPtr TaintTracer::CreateTrace() {
  TAINT_OBSERVER_CAP_STACK();

  TaintTraceDataPtr head;
  Array bt = FrameInjection::GetBacktrace();
  ArrayIter it(bt);

  Array frame;

  if (it) {
    frame = it.second().toArray();
    head = NEW(TaintTraceData)(TraceFrameAsString(frame, 0));
    ++it;
  }

  TaintTraceDataPtr ttd = head;
  for (int i = 1; it; ++it, ++i) {
    frame = it.second().toArray();
    ttd = ttd->attachData(TraceFrameAsString(frame, i));
  }

  return head;
}

std::string TaintTracer::ExtractTrace(const TaintTraceNodePtr& root) {
  TAINT_OBSERVER_CAP_STACK();

  hphp_string_set sourceset;
  hphp_string_set frameset;

  ExtractInternal(root, sourceset, frameset);

  int i;
  hphp_string_set::iterator it;
  std::stringstream ss;

  ss << "source strings fragments:\n";
  if (sourceset.empty()) {
    ss << "    (none)\n";
    ss << "NB: Source strings are not kept for file reads; they are also ";
    ss << "dropped when passed into the APC.\n";
  }
  for (i = 0, it = sourceset.begin(); it != sourceset.end(); ++i, ++it) {
    ss << "    #" << i << " ";
    ss << *it << "\n";
  }

  for (i = 0, it = frameset.begin(); it != frameset.end(); ++i, ++it) {
    if (i > 0) {
      ss << "\n";
    }
    ss << "trace{" << i << "}:\n";
    ss << *it;
  }

  return ss.str();
}

void TaintTracer::ExtractInternal(const TaintTraceNodePtr& root,
    hphp_string_set& sourceset, hphp_string_set& frameset) {
  TaintTraceNodePtr node;
  TaintTraceDataPtr ttd;

  for (node = root; node.get(); node = node->getNext()) {
    if (node->getChild().get()) {
      assert(!node->getLeaf().get());
      ExtractInternal(node->getChild(), sourceset, frameset);
    } else if ((ttd = node->getLeaf()).get()) {
      if (strncmp(ttd->getStr().c_str(), "    ", 4) != 0) {
        // Naive heuristic for determinine if we're a stacktrace leaf.
        sourceset.insert(ttd->getStr().c_str());
      } else {
        std::string buf;
        for ( ; ttd.get(); ttd = ttd->getNext()) {
          buf += ttd->getStr().c_str();
        }
        frameset.insert(buf);
      }
    } else {
      assert(false);
    }
  }
}

}

#endif // TAINTED
