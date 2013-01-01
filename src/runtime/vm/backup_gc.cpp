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
#include "runtime/vm/backup_gc.h"

#include <fstream>
#include <algorithm>
#include <boost/format.hpp>
#include <boost/noncopyable.hpp>
#include <map>

#include "util/assertions.h"
#include "util/timer.h"
#include "util/trace.h"
#include "runtime/base/execution_context.h"
#include "runtime/base/memory/smart_allocator.h"
#include "runtime/base/memory/memory_manager.h"
#include "runtime/base/complex_types.h"
#include "runtime/base/array/hphp_array.h"
#include "runtime/vm/class.h"

namespace HPHP { namespace VM {

static const Trace::Module TRACEMOD = Trace::gc;

//////////////////////////////////////////////////////////////////////

namespace {

enum Color {
  Colorless,
  Black,      // Known to be reachable
  Garbage     // Used for GarbageDetector
};

typedef hphp_hash_map<void*,Color> ColorMap;
typedef std::pair<DataType,void*> TypedObj;
typedef std::set<TypedObj> TypedObjSet;

TypedObj make_typed_obj(RefData* p) {
  return std::make_pair(KindOfRef, p);
}
TypedObj make_typed_obj(ObjectData* p) {
  return std::make_pair(KindOfObject, p);
}
TypedObj make_typed_obj(ArrayData* p) {
  return std::make_pair(KindOfArray, p);
}

struct GCState : private boost::noncopyable {
  GCState()
    : m_totalCount(0)
    , m_collectedCount(0)
  {}

  ColorMap m_colorMap;
  uint64_t m_totalCount;
  uint64_t m_collectedCount;

  // For cycle detection, we keep a set of all the leaked heap objects
  // here to make it easier to emit nodes and edges without doing it
  // during a heap iteration.
  TypedObjSet m_cyclicGarbage;

  // Set the color of an object to newColor, and return whatever its
  // old color was.
  Color setColor(void* vp, Color newColor) {
    std::pair<ColorMap::iterator,bool> p =
      m_colorMap.insert(std::make_pair(vp, newColor));
    Color ret = !p.second ? p.first->second : Colorless;
    p.first->second = newColor;
    return ret;
  }
};

int32_t* count_addr(void* obj) {
  void* addr = static_cast<char*>(obj) + FAST_REFCOUNT_OFFSET;
  return static_cast<int32_t*>(addr);
}

bool is_smart_allocated(ObjectData* obj) {
  // XXX ObjectData is allocated from several smart allocators by
  // size, so currently we have to check them all.
  MemoryManager::AllocIterator aIter(MemoryManager::TheMemoryManager());
  while (SmartAllocatorImpl* sa = aIter.current()) {
    if (sa->getAllocatorType() == SmartAllocatorImpl::ObjectData) {
      if (sa->isFromThisAllocator(obj)) return true;
    }
    aIter.next();
  }
  return false;
}

template<class T>
bool UNUSED is_smart_allocated(T* p) {
  return T::AllocatorType::getNoCheck()->isFromThisAllocator(p);
}

bool is_static(void* obj) {
  return *count_addr(obj) == RefCountStaticValue;
}

template<class ObjectType, class Visitor>
void walk_allocator(const Visitor& visit, SmartAllocatorImpl* sa) {
  SmartAllocatorImpl::Iterator saIter(sa);
  while (ObjectType* p = static_cast<ObjectType*>(saIter.current())) {
    visit(sa, p);
    saIter.next();
  }
}

/*
 * Iterate every live object in any smart-allocator, and call visit()
 * on it.
 *
 * The collection algorithm involves several heap walks (see
 * collect_algorithm below).
 */
template<class Visitor>
void walk_smart_heap(const Visitor& visit) {
  MemoryManager::AllocIterator aIter(MemoryManager::TheMemoryManager());
  while (SmartAllocatorImpl* sa = aIter.current()) {
    switch (sa->getAllocatorType()) {
    case SmartAllocatorImpl::HphpArray:
      walk_allocator<ArrayData>(visit, sa);
      break;
    case SmartAllocatorImpl::RefData:
      walk_allocator<RefData>(visit, sa);
      break;
    case SmartAllocatorImpl::ObjectData:
      walk_allocator<ObjectData>(visit, sa);
      break;
    case SmartAllocatorImpl::StringData:
      // Unneccesary for the first level walk, because strings can't
      // have references to other objects.
      break;
    case SmartAllocatorImpl::ZendArray:
    case SmartAllocatorImpl::VectorArray:
      /*
       * Currently unimplemented.
       *
       * Note: ZendArray's sweep doesn't decref its Buckets (because
       * they might be sweeping), so we'd have to add that to the
       * deallocation code.  VectorArray is unimplemented here simply
       * because it's not tested, and unused in the vm anyway.
       */
      break;
    default:
      break;
    }
    aIter.next();
  }
}

template<class Visitor> void traceImpl(const Visitor&, ArrayData*);
template<class Visitor> void traceImpl(const Visitor&, ObjectData*);
template<class Visitor> void traceImpl(const Visitor&, RefData*);

// This just exists so that visitors that don't want to visit strings
// don't need to have an instantiable operator() for a StringData*
// argument.
template<bool DoIt, class Visitor>
struct VisitStringHelper {
  static void visit(const Visitor& visit, StringData* sd) {
    visit(sd);
  }
};
template<class Visitor>
struct VisitStringHelper<false,Visitor> {
  static void visit(const Visitor&, StringData*) {}
};

template<class Visitor>
void traceImpl(const Visitor& visit, TypedValue* tv) {
  switch (tv->m_type) {
  case KindOfRef:
    ASSERT(!is_static(tv->m_data.pref));
    ASSERT(is_smart_allocated(tv->m_data.pref));
    visit(tv->m_data.pref);
    break;
  case KindOfObject:
    /*
     * We need to check whether ObjectData*'s are actually from the
     * smart heap before tracing them (some ResourceData objects are
     * not smart allocated).
     *
     * Since we don't trace them, this means that garbage cycles
     * involving non-smart allocated extension objects can't be
     * collected.  On the other hand, if a garbage cycle has a
     * reference to one of these ResourceData objects, when we collect
     * it we will leave the reference to it despite collecting the
     * cycle (this is the same as what would happen if we let the
     * cycle leak, so this is semantically ok although it is a
     * resource leak).
     */
    ASSERT(!is_static(tv->m_data.pref));
    if (is_smart_allocated(tv->m_data.pobj)) {
      visit(tv->m_data.pobj);
    }
    break;
  case KindOfArray:
    if (!is_static(tv->m_data.parr)) {
      visit(tv->m_data.parr);
    }
    break;
  case KindOfString:
    if (!is_static(tv->m_data.pstr)) {
      ASSERT(is_smart_allocated(tv->m_data.pstr));
      VisitStringHelper<Visitor::visits_strings,Visitor>::visit(
        visit,
        tv->m_data.pstr
      );
    }
    break;
  default:
    break;
  }
}

template<class Visitor>
void traceImpl(const Visitor& visit, ArrayData* ad) {
  for (ssize_t i = ad->iter_begin();
      i != ArrayData::invalid_index;
      i = ad->iter_advance(i)) {
    /*
     * We need to visit the keys only if they are strings (for the
     * case where we are deallocating objects).  The ArrayData api
     * adds references when looking at keys, but this is ok because
     * GarbageCollector doesn't actually free StringData's outright
     * (just decrefs them).
     */
    if (Visitor::visits_strings) {
      Variant key(ad->getKey(i));
      if (key.isString()) {
        traceImpl(visit, key.asTypedValue());
      } else {
        ASSERT(key.isInteger());
      }
    }

    // The key is either a string or an int, so we don't care.  Only
    // look at the value.  Trace the TypedValue so we visit whatever
    // it points to.
    traceImpl(visit, const_cast<TypedValue*>(
      ad->getValueRef(i).asTypedValue()));
  }
}

template<class Visitor>
void traceImpl(const Visitor& visit, ObjectData* obj) {
  // Dynamic properties.  We have to visit the dynamic property array
  // itself, since it is an object living in the smart heap (all the
  // top-level walks visit it anyway).
  if (ArrayData* dyn = obj->getProperties().get()) {
    visit(dyn);
  }

  // Declared properties.  We need to indirect through the TypedValue
  // before visiting, since these are in-situ in the VM::Instance.
  void* vpObj = obj;
  unsigned char* address = static_cast<unsigned char*>(vpObj);

  const size_t nProps = obj->getVMClass()->numDeclProperties();
  for (size_t i = 0; i < nProps; ++i) {
    size_t off = obj->getVMClass()->declPropOffset(i);

    void* tvAddr = address + off;
    traceImpl(visit, static_cast<TypedValue*>(tvAddr));
  }
}

template<class Visitor>
void traceImpl(const Visitor& visit, RefData* ref) {
  visit(ref);
}

template<class Visitor, class T>
void trace(const Visitor& visit, T* t) {
  traceImpl(visit, t);
}

template<class Visitor>
void trace(const Visitor& visit, RefData* ref) {
  traceImpl(visit, ref->tv());
}

struct RefDecrement {
  static const bool visits_strings = false;

  template<class T>
  void operator()(T* t) const {
    --*count_addr(t);
    ASSERT(*count_addr(t) >= 0);
  }
};

struct RefIncrement {
  static const bool visits_strings = false;

  template<class T>
  void operator()(T* t) const {
    ASSERT(*count_addr(t) >= 0);
    ++*count_addr(t);
  }
};

struct InternalRefRemover {
  template<class T>
  void operator()(SmartAllocatorImpl*, T* t) const {
    trace(RefDecrement(), t);
  }
};

struct MarkLive {
  // Since we can't trace StringData, we don't need to bother marking
  // these.
  static const bool visits_strings = false;

  explicit MarkLive(GCState& state) : m_state(state) {}

  template<class T>
  void operator()(T* t) const {
    ++*count_addr(t);
    if (m_state.setColor(t, Black) == Colorless) {
      trace(*this, t);
    }
  }

  GCState& m_state;
};

struct ExternalRefRestorer {
  explicit ExternalRefRestorer(GCState& state) : m_state(state) {}

  template<class T> void operator()(SmartAllocatorImpl*, T* t) const {
    if (*count_addr(t) == 0) return;
    if (m_state.setColor(t, Black) == Colorless) {
      trace(MarkLive(m_state), t);
    }
  }

  GCState& m_state;
};

/*
 * Since strings haven't been involved in the heap walk stuff, they
 * won't get freed when they are part of a cycle unless we handle them
 * specially here.  It's only necessary to trace one level out looking
 * for strings---any strings involved in the cycle are at most one
 * level out from an object that GarbageCollector will visit.
 * Moreover, if a string is shared, we might visit it more than once
 * here---so just decref it, don't free it.
 */
struct StringDealloc {
  static const bool visits_strings = true;

  void operator()(StringData* s) const { decRefStr(s); }
  template<class T> void operator()(T*) const {}
};

struct GarbageCollector {
  explicit GarbageCollector(GCState& state) : m_state(state) {}

  /*
   * Deallocation for these objects needs to go directly to the
   * SmartAllocator.  If we try to run their destructors or release
   * functions, they'll try to decref the things they refer to, even
   * though all those things have a zero _count now.  We also don't
   * want to run object destructors.
   *
   * Also, HphpArray and some objects have to be swept before we tell
   * the allocator it's done.
   */
  template<class T>
  void operator()(SmartAllocatorImpl* sa, T* t) const {
    const int32_t count = *count_addr(t);
    if (!count) {
      trace(StringDealloc(), t);
      dealloc(sa, t);
      ++m_state.m_collectedCount;
    }
    ++m_state.m_totalCount;
  }

  void dealloc(SmartAllocatorImpl* sa, ArrayData* ar) const {
    if (Sweepable* s = dynamic_cast<Sweepable*>(ar)) {
      s->sweep();
      s->unregister();
    }
    sa->dealloc(ar);
  }

  void dealloc(SmartAllocatorImpl* sa, ObjectData* obj) const {
    if (Sweepable* s = dynamic_cast<Sweepable*>(obj)) {
      s->sweep();
      s->unregister();
    }
    if (RuntimeOption::EnableObjDestructCall) {
      g_vmContext->m_liveBCObjs.erase(obj);
    }
    sa->dealloc(obj);
  }

  void dealloc(SmartAllocatorImpl* sa, RefData* rd) const {
    sa->dealloc(rd);
  }

  GCState& m_state;
};

struct GarbageDetector {
  explicit GarbageDetector(GCState& state) : m_state(state) {}

  template<class T>
  void operator()(SmartAllocatorImpl*, T* t) const {
    // Check a color flag instead of the count, because we might
    // increment it before seeing it.
    if (m_state.setColor(t, Garbage) == Colorless) {
      m_state.m_cyclicGarbage.insert(make_typed_obj(t));

      /*
       * To leave the heap in a consistent state, we need to add back all
       * the internal references for the garbage.
       */
      trace(RefIncrement(), t);
    }
  }

  GCState& m_state;
};

/*
 * Implements a garbage collection/detection algorithm that doesn't
 * require knowing about every reference in the system.
 *
 * References outside of the set of objects we evaluate (the smart
 * heap) will just keep objects alive.  (This means a cycle living
 * partially in native C++ objects won't be collectable.)
 */
template<class FinalStep>
void collect_algorithm(GCState& state) {
  /*
   * Step 1:
   *
   *   Walk the entire heap, and for each object, subtract one from
   *   the reference counts of all objects it refers to.
   *
   *   We do this to objects that can have references to other
   *   objects, but ignore strings.
   */
  walk_smart_heap(InternalRefRemover());

  /*
   * Step 2:
   *
   *   Each (non-string) object that still has a non-zero reference
   *   count has references that live outside of the smart heap, and
   *   therefore is live.  Mark it live, and mark every object it
   *   refers to (transitively) live.
   */
  walk_smart_heap(ExternalRefRestorer(state));

  /*
   * Step 3:
   *
   *   Any object that still has a zero reference count is part of a
   *   heap-internal cycle.  Do something with it.
   */
  walk_smart_heap(FinalStep(state));
}

struct EdgePrinter {
  static const bool visits_strings = false;

  explicit EdgePrinter(hphp_hash_map<void*,uint32_t>& nodeIds,
                       uint32_t srcId,
                       std::ostream& out)
    : m_nodeIds(nodeIds)
    , m_srcId(srcId)
    , m_out(out)
  {}

  template<class T> void operator()(T* t) const {
    // TODO: could show which member or array key pointed to this?
    m_out << "  edge [\n"
             "    source " << m_srcId << '\n'
          << "    target " << m_nodeIds[t] << '\n'
          << "  ]\n";
  }

  hphp_hash_map<void*,uint32_t>& m_nodeIds;
  uint32_t m_srcId;
  std::ostream& m_out;
};

}

//////////////////////////////////////////////////////////////////////

std::string gc_collect_cycles() {
  TRACE(1, "GC: starting gc_collect_cycles\n");

  Timer cpuTimer(Timer::TotalCPU);
  Timer wallTimer(Timer::WallTime);

  GCState state;
  collect_algorithm<GarbageCollector>(state);

  const uint64_t live = state.m_totalCount - state.m_collectedCount;
  const float survivalRate = 100 * float(live) /
                             std::max(state.m_totalCount, 1ul);
  std::string ret = str(
    boost::format("released %d/%d objects; survival%% = %02.2f; "
                  "cpu time = %5lld; wall time = %5lld\n")
      % state.m_collectedCount
      % state.m_totalCount
      % survivalRate
      % cpuTimer.getMicroSeconds()
      % wallTimer.getMicroSeconds());
  TRACE(1, "%s", ret.c_str());
  return ret;
}

void gc_detect_cycles(const std::string& filename) {
  TRACE(1, "GC: starting gc_detect_cycles\n");

  GCState state;
  collect_algorithm<GarbageDetector>(state);

  std::ofstream out(filename.c_str());
  if (!out.is_open()) {
    raise_error("couldn't open output file for gc_detect_cycles, %s",
                strerror(errno));
    return;
  }

  uint32_t nextNodeId = 1;
  hphp_hash_map<void*,uint32_t> nodeIds;

  out << "graph [\n"
         "  directed 1\n";

  // Print nodes.
  for (TypedObjSet::const_iterator it = state.m_cyclicGarbage.begin();
      it != state.m_cyclicGarbage.end();
      ++it) {
    uint32_t thisNodeId = nextNodeId++;
    nodeIds[it->second] = thisNodeId;

    const char* name;
    const char* color;
    switch (it->first) {
    case KindOfObject: {
      ObjectData* od = static_cast<ObjectData*>(it->second);
      name = od->getVMClass()->nameRef().data();
      color = "#FFCC00";
      break;
    }
    case KindOfArray:
      name = "array()";
      color = "#CCCCFF";
      break;
    case KindOfRef:
      name = "RefData";
      color = "#33CCCC";
      break;
    default:
      not_reached();
    }
    out << "  node [ id " << thisNodeId << "\n"
           "    graphics [\n"
           "      type \"roundrectangle\"\n"
           "      fill \"" << color << "\"\n"
           "    ]\n"
           "    LabelGraphics [\n"
           "      anchor \"e\"\n"
           "      alignment \"left\"\n"
           "      fontName \"Consolas\"\n"
           "      text \"" << name << "\"\n"
           "    ]\n"
           "  ]\n";
  }

  // Print edges.
  for (TypedObjSet::const_iterator it = state.m_cyclicGarbage.begin();
      it != state.m_cyclicGarbage.end();
      ++it) {
    EdgePrinter p(nodeIds, nodeIds[it->second], out);
    switch (it->first) {
    case KindOfObject:
      trace(p, static_cast<ObjectData*>(it->second));
      break;
    case KindOfRef:
      trace(p, static_cast<RefData*>(it->second));
      break;
    case KindOfArray:
      trace(p, static_cast<ArrayData*>(it->second));
      break;
    default:
      ASSERT(false);
    }
  }

  out << "]\n";

  TRACE(1, "GC: %zu objects were part of cycles; wrote to %s\n",
           state.m_cyclicGarbage.size(),
           filename.c_str());
}

//////////////////////////////////////////////////////////////////////

}}

