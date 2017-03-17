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

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/builtin-functions.h"

/*
 *                       Heapgraph Extension
 * What is it?
 * Set of methods to wrap around HHVM's heap graph implementation
 *
 * How does it work?
 * Create a heap graph in HHVM and uses it as a Resource with a
 * set of functions that can operate on it.
 *
 * How do I use it?
 * Call heapgraph_create, and then any of the other heapgraph
 * function
 */

#include <set>
#include <unordered_map>
#include <vector>
#include <boost/dynamic_bitset.hpp>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/memory-manager-defs.h"
#include "hphp/runtime/ext/datetime/ext_datetime.h"
#include "hphp/runtime/ext/simplexml/ext_simplexml.h"
#include "hphp/runtime/ext/std/ext_std_closure.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/util/alloc.h"
#include "hphp/runtime/base/heap-graph.h"
#include "hphp/runtime/base/heap-algorithms.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {

struct PhpStack;
struct CppStack;
struct CppTls;

namespace {

TRACE_SET_MOD(heapgraph);

///////////////////////////////////////////////////////////////////////////////

const StaticString
  s_nodes("nodes"),
  s_edges("edges"),
  s_roots("roots"),
  s_root_nodes("root_nodes"),
  s_root_path("root_path"),
  s_exact("exact"),

  // Node description
  s_kind("kind"),
  s_size("size"),
  s_index("index"),
  s_class("class"),

  // Edge description
  s_seat("seat"),
  s_name("name"),
  s_from("from"),
  s_to("to");

}

///////////////////////////////////////////////////////////////////////////////
// CONTEXT OBJECTS

// Extra information about a HeapGraph::Node.
struct CapturedNode {
  HeaderKind kind;
  LowPtr<Class> cls;
};

// Extra information about a HeapGraph::Ptr
struct CapturedPtr {
  std::string edgename;
};

struct HeapGraphContext : SweepableResourceData {
  explicit HeapGraphContext(const HeapGraph& hg) : hg(hg) {}
  explicit HeapGraphContext(HeapGraph&& hg) : hg(std::move(hg)) {}
  ~HeapGraphContext() {}

  bool isInvalid() const override {
    return false;
  }

  CLASSNAME_IS("HeapGraphContext")
  DECLARE_RESOURCE_ALLOCATION(HeapGraphContext)

  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

  std::vector<CapturedNode> cnodes;
  std::vector<CapturedPtr> cptrs;
  const HeapGraph hg;
};

IMPLEMENT_RESOURCE_ALLOCATION(HeapGraphContext)

namespace {

using HeapGraphContextPtr = req::ptr<HeapGraphContext>;
static HeapGraphContextPtr get_valid_heapgraph_context_resource(
  const Resource& resource,
  const char* func_name
) {
  auto hgcontext = dyn_cast_or_null<HeapGraphContext>(resource);
  if (hgcontext == nullptr || hgcontext->isInvalid()) {
    raise_warning(
      "%s(): supplied resource is not a valid HeapGraph Context resource",
      func_name + 2
    );
    return nullptr;
  }
  return hgcontext;
}

///////////////////////////////////////////////////////////////////////////////
// TRAVERSAL FUNCTIONS

bool supportsToArray(const ObjectData* obj) {
  if (obj->isCollection()) {
    assertx(isValidCollection(obj->collectionType()));
    return true;
  } else if (UNLIKELY(obj->getAttribute(ObjectData::CallToImpl))) {
    return obj->instanceof(SimpleXMLElement_classof());
  } else if (UNLIKELY(obj->instanceof(SystemLib::s_ArrayObjectClass))) {
    return true;
  } else if (UNLIKELY(obj->instanceof(SystemLib::s_ArrayIteratorClass))) {
    return true;
  } else if (UNLIKELY(obj->instanceof(c_Closure::classof()))) {
    return true;
  } else if (UNLIKELY(obj->instanceof(DateTimeData::getClass()))) {
    return true;
  } else {
    if (LIKELY(!obj->hasInstanceDtor())) {
      return true;
    }

    return false;
  }
}

std::string getObjectConnectionName(
  const ObjectData* obj,
  const void* target
) {
  Class* cls = obj->getVMClass();
  FTRACE(5, "HG: Getting connection name for type {} at {}\n",
    obj->getClassName().data(),
    obj
  );

  if (!supportsToArray(obj)) {
    return "";
  }

  auto arr = obj->toArray(); // TODO t12985984 avoid toArray.
  bool is_packed = arr->hasPackedLayout();
  for (ArrayIter iter(arr); iter; ++iter) {
    auto first = iter.first();
    auto key = first.toString();
    auto key_tv = first.asTypedValue();
    auto val_tv = iter.secondRef().asTypedValue();

    if (isStringType(key_tv->m_type)) {
      // If the key begins with a NUL, it's a private or protected property.
      // Read the class name from between the two NUL bytes.
      //
      // Note: Copied from object-data.cpp
      if (!key.empty() && key[0] == '\0') {
        int subLen = key.find('\0', 1) + 1;
        key = key.substr(subLen);
      }
    }

    FTRACE(5, "HG: ...Iterating over object key-val {}=>{}\n",
      key, tname(val_tv->m_type)
    );

    // We're only interested in the property name that points to our target
    if ((void*)val_tv->m_data.pobj != target) {
      continue;
    }

    bool is_declared = isStringType(key_tv->m_type) &&
                       cls->lookupDeclProp(key.get()) != kInvalidSlot;

    if (!is_declared && !is_packed) {
      return std::string("Key:" + key);
    } else if (is_packed) {
      return std::string("PropertyIndex");
    } else {
      return std::string("Property:" + key);
    }
  }

  return "";
}

static const StringData* edge_kind_strs[3];
static const char* edge_kind_cstrs[] = {
  "Ptr:Counted", "Ptr:Implicit", "Ptr:Ambiguous"
};

const StringData* edgeKindName(HeapGraph::PtrKind kind) {
  auto s = edge_kind_strs[(int)kind];
  if (!s) {
    s = makeStaticString(edge_kind_cstrs[(int)kind]);
    edge_kind_strs[(int)kind] = s;
  }
  return s;
  static_assert(HeapGraph::NumPtrKinds == 3, "");
  static_assert(HeapGraph::Counted == 0, "");
  static_assert(HeapGraph::Implicit == 1, "");
  static_assert(HeapGraph::Ambiguous == 2, "");
}

std::string rootPtrDescription(const HeapGraph::Node& node) {
  if (!node.is_root || !type_scan::hasNonConservative()) return "";
  auto ty = node.tyindex;
  if (ty == type_scan::getIndexForScan<rds::StaticLocalData>()) {
    return "RdsNormal";
  }
  if (ty == type_scan::getIndexForScan<StaticPropData>()) {
    return "RdsLocal";
  }
  if (ty == type_scan::getIndexForScan<rds::Header>()) {
    return "RdsHeader";
  }
  if (ty == type_scan::getIndexForScan<PhpStack>()) {
    return "PhpStack";
  }
  if (ty == type_scan::getIndexForScan<CppStack>()) {
    return "CppStack";
  }
  if (ty == type_scan::getIndexForScan<CppTls>()) {
    return "CppTls";
  }
  return "";
}

std::string getNodesConnectionName(const HeapGraph& g, int ptr) {
  assert(g.ptrs[ptr].from != -1 && g.ptrs[ptr].to != -1);
  auto& edge = g.ptrs[ptr];
  auto& from = g.nodes[edge.from];
  // Try to drill down and resolve the edge name
  if (!from.is_root) {
    auto h = from.h;
    auto th = g.nodes[edge.to].h;

    // get the from/to object, if any. this deals with object kinds that
    // have data before the object: AFWH, Native, and Closure.
    auto h_obj = h->obj();
    auto th_obj = th->obj();

    switch (h->kind()) {
      // Known generalized cases that don't really need pointer kind
      case HeaderKind::Mixed:
      case HeaderKind::Dict:
      case HeaderKind::Keyset:
        return "ArrayKeyValue";

      // Obvious cases that do not need pointer type
      case HeaderKind::AwaitAllWH:
      case HeaderKind::WaitHandle:
      case HeaderKind::AsyncFuncWH:
      case HeaderKind::Pair:
      case HeaderKind::AsyncFuncFrame:
      case HeaderKind::Set:
      case HeaderKind::ImmSet:
      case HeaderKind::Vector:
      case HeaderKind::ImmVector:
      case HeaderKind::Packed:
      case HeaderKind::VecArray:
        return "";

      // Explicit cases that have explicit pointer name

      case HeaderKind::Ref:
        return "";

      case HeaderKind::Map:
      case HeaderKind::ImmMap:
      case HeaderKind::ClosureHdr:
      case HeaderKind::Closure:
      case HeaderKind::NativeData:
      case HeaderKind::Object: {
        auto conn_name = getObjectConnectionName(h_obj,
            th_obj ? static_cast<const void*>(th_obj) :
            static_cast<const void*>(th)
        );
        if (!conn_name.empty()) {
          return conn_name;
        }
        // Fallback to pointer kind
        break;
      }

      // Unknown drilldown cases that need pointer type
      case HeaderKind::Empty:
      case HeaderKind::Apc:
      case HeaderKind::Globals:
      case HeaderKind::Proxy:
      case HeaderKind::String:
      case HeaderKind::Resource:
      case HeaderKind::BigMalloc:
      case HeaderKind::SmallMalloc:
      case HeaderKind::Free:
      case HeaderKind::BigObj:
      case HeaderKind::Hole:
        // Fallback to pointer kind
        break;
    }
  } else {
    // ptr from root node
    return rootPtrDescription(from);
  }

  return edgeKindName(g.ptrs[ptr].ptr_kind)->data();
}

void heapgraphCallback(Array fields, const Variant& callback) {
  VMRegAnchor _;
  auto params = make_packed_array(fields);
  vm_call_user_func(callback, params);
}

void heapgraphCallback(Array fields, Array fields2, const Variant& callback) {
  VMRegAnchor _;
  auto params = make_packed_array(fields, fields2);
  vm_call_user_func(callback, params);
}

static const StringData* header_name_strs[NumHeaderKinds];

Array createPhpNode(HeapGraphContextPtr hgptr, int index) {
  const auto& node = hgptr->hg.nodes[index];
  const auto& cnode = hgptr->cnodes[index];

  if (node.is_root) {
    return make_map_array(
      s_index, Variant(index),
      s_kind, VarNR(makeStaticString(type_scan::getName(node.tyindex))),
      s_size, Variant(int64_t(node.size))
    );
  }

  auto kind_str = header_name_strs[int(cnode.kind)];
  if (!kind_str) {
    kind_str = makeStaticString(header_names[int(cnode.kind)]);
    header_name_strs[int(cnode.kind)] = kind_str;
  }

  auto node_arr = make_map_array(
    s_index, Variant(index),
    s_kind, VarNR(kind_str),
    s_size, Variant(int64_t(node.size))
  );

  if (cnode.cls) {
    node_arr.set(s_class, VarNR(cnode.cls->nameStr()));
  }
  return node_arr;
}

Array createPhpEdge(HeapGraphContextPtr hgptr, int index) {
  const auto& ptr = hgptr->hg.ptrs[index];
  const auto& cptr = hgptr->cptrs[index];

  auto ptr_arr = make_map_array(
    s_index, Variant(index),
    s_kind, VarNR(edgeKindName(ptr.ptr_kind)),
    s_from, Variant(ptr.from),
    s_to, Variant(ptr.to),
    s_seat, VarNR(
      makeStaticString(rootPtrDescription(hgptr->hg.nodes[ptr.from]))
    ),
    s_name, Variant(cptr.edgename)
  );

  return ptr_arr;
}

std::vector<int> toBoundIntVector(const Array& arr, int64_t max) {
  std::vector<int> result;
  result.reserve(arr.size());
  for (ArrayIter iter(arr); iter; ++iter) {
    auto index = iter.second().toInt64(); // Cannot re-enter.
    if (index < 0 || index >= max) {
      continue;
    }

    result.push_back(index);
  }
  return result;
}

///////////////////////////////////////////////////////////////////////////////
// Exports

Resource HHVM_FUNCTION(heapgraph_create, void) {
  HeapGraph hg = makeHeapGraph();
  std::vector<CapturedNode> cnodes;
  std::vector<CapturedPtr> cptrs;

  // Copy edges into captured edges
  // Capturing edges first because after capturing nodes we nullify the header
  cptrs.reserve(hg.ptrs.size());
  for (int i = 0; i < hg.ptrs.size(); ++i) {
    auto new_ptr = CapturedPtr{
      /* edgename */ getNodesConnectionName(hg, i)
    };
    cptrs.push_back(new_ptr);
  }

  // Copy nodes into captured nodes
  cnodes.reserve(hg.nodes.size());
  for (int i = 0; i < hg.nodes.size(); ++i) {
    const auto& src_node = hg.nodes[i];
    if (src_node.is_root) {
      cnodes.push_back(CapturedNode{
        /* kind */      (HeaderKind)NumHeaderKinds,
        /* classname */ nullptr
      });
    } else {
      auto obj = src_node.h->obj();
      cnodes.push_back(CapturedNode{
        /* kind */      src_node.h->kind(),
        /* classname */ obj ? obj->getVMClass() : nullptr
      });
    }

    // Nullify the pointers to be safe since this is a captured heap
    hg.nodes[i].h = nullptr;
  }

  auto hgcontext = req::make<HeapGraphContext>(std::move(hg));
  std::swap(hgcontext->cnodes, cnodes);
  std::swap(hgcontext->cptrs, cptrs);
  return Resource(hgcontext);
}

void HHVM_FUNCTION(heapgraph_foreach_node,
  const Resource& resource,
  const Variant& callback
) {
  auto hgptr = get_valid_heapgraph_context_resource(resource, __FUNCTION__);
  if (!hgptr || callback.isNull()) return;
  for (int i = 0; i < hgptr->hg.nodes.size(); i++) {
    auto phpnode = createPhpNode(hgptr, i);
    heapgraphCallback(phpnode, callback);
  }
}

void HHVM_FUNCTION(heapgraph_foreach_edge,
  const Resource& resource,
  const Variant& callback
) {
  auto hgptr = get_valid_heapgraph_context_resource(resource, __FUNCTION__);
  if (!hgptr || callback.isNull()) return;
  for (int i = 0; i < hgptr->hg.ptrs.size(); i++) {
    auto phpedge = createPhpEdge(hgptr, i);
    heapgraphCallback(phpedge, callback);
  }
}

void HHVM_FUNCTION(heapgraph_foreach_root,
  const Resource& resource,
  const Variant& callback
) {
  auto hgptr = get_valid_heapgraph_context_resource(resource, __FUNCTION__);
  if (!hgptr || callback.isNull()) return;
  for (int i = 0, n = hgptr->hg.root_ptrs.size(); i < n; ++i) {
    auto phpedge = createPhpEdge(hgptr, hgptr->hg.root_ptrs[i]);
    heapgraphCallback(phpedge, callback);
  }
}

void HHVM_FUNCTION(heapgraph_dfs_nodes,
  const Resource& resource,
  const Array& roots_arr,
  const Array& skips_arr,
  const Variant& callback
) {
  auto hgptr = get_valid_heapgraph_context_resource(resource, __FUNCTION__);
  if (!hgptr || callback.isNull()) return;
  auto max = hgptr->hg.nodes.size();
  auto roots = toBoundIntVector(roots_arr, max);
  auto skips = toBoundIntVector(skips_arr, max);
  dfs_nodes(hgptr->hg, roots, skips, [&](int n) {
    auto phpnode = createPhpNode(hgptr, n);
    heapgraphCallback(phpnode, callback);
  });
}

void HHVM_FUNCTION(heapgraph_dfs_edges,
  const Resource& resource,
  const Array& roots_arr,
  const Array& skips_arr,
  const Variant& callback
) {
  auto hgptr = get_valid_heapgraph_context_resource(resource, __FUNCTION__);
  if (!hgptr || callback.isNull()) return;
  auto max = hgptr->hg.ptrs.size();
  auto roots = toBoundIntVector(roots_arr, max);
  auto skips = toBoundIntVector(skips_arr, max);
  dfs_ptrs(hgptr->hg, roots, skips, [&](int n, int p) {
    auto phpnode = createPhpNode(hgptr, n);
    auto phpedge = createPhpEdge(hgptr, p);
    heapgraphCallback(phpedge, phpnode, callback);
  });
}

Array HHVM_FUNCTION(heapgraph_edge, const Resource& resource, int64_t index) {
  auto hgptr = get_valid_heapgraph_context_resource(resource, __FUNCTION__);
  if (!hgptr) return empty_array();
  if (size_t(index) >= hgptr->hg.ptrs.size()) return empty_array();
  return createPhpEdge(hgptr, index);
}

Array HHVM_FUNCTION(heapgraph_node, const Resource& resource, int64_t index) {
  auto hgptr = get_valid_heapgraph_context_resource(resource, __FUNCTION__);
  if (!hgptr) return empty_array();
  if (size_t(index) >= hgptr->hg.nodes.size()) return empty_array();
  return createPhpNode(hgptr, index);
}

Array HHVM_FUNCTION(heapgraph_node_out_edges,
  const Resource& resource,
  int64_t index
) {
  auto hgptr = get_valid_heapgraph_context_resource(resource, __FUNCTION__);
  if (!hgptr) return empty_array();
  if (size_t(index) >= hgptr->hg.nodes.size()) return empty_array();
  size_t num_edges{0};
  hgptr->hg.eachOutPtr(index, [&](int) { num_edges++; });
  PackedArrayInit result(num_edges);
  hgptr->hg.eachOutPtr(index, [&](int ptr) {
    result.append(createPhpEdge(hgptr, ptr));
  });
  return result.toArray();
}

Array HHVM_FUNCTION(heapgraph_node_in_edges,
  const Resource& resource,
  int64_t index
) {
  auto hgptr = get_valid_heapgraph_context_resource(resource, __FUNCTION__);
  if (!hgptr) return empty_array();
  if (size_t(index) >= hgptr->hg.nodes.size()) return empty_array();
  size_t num_edges{0};
  hgptr->hg.eachInPtr(index, [&](int) { num_edges++; });
  PackedArrayInit result(num_edges);
  hgptr->hg.eachInPtr(index, [&](int ptr) {
    result.append(createPhpEdge(hgptr, ptr));
  });
  return result.toArray();
}

Array HHVM_FUNCTION(heapgraph_stats, const Resource& resource) {
  auto hgptr = get_valid_heapgraph_context_resource(resource, __FUNCTION__);
  if (!hgptr) return empty_array();
  auto result = make_map_array(
    s_nodes, Variant(hgptr->hg.nodes.size()),
    s_edges, Variant(hgptr->hg.ptrs.size()),
    s_roots, Variant(hgptr->hg.root_ptrs.size()),
    s_root_nodes, Variant(hgptr->hg.root_nodes.size()),
    s_exact, Variant(type_scan::hasNonConservative())
  );
  return result;
}

///////////////////////////////////////////////////////////////////////////////
// Extension

}

struct heapgraphExtension final : Extension {
  heapgraphExtension() : Extension("heapgraph", "1.0") { }

  void moduleInit() override {
    HHVM_FALIAS(HH\\heapgraph_create, heapgraph_create);
    HHVM_FALIAS(HH\\heapgraph_stats, heapgraph_stats);
    HHVM_FALIAS(HH\\heapgraph_foreach_node, heapgraph_foreach_node);
    HHVM_FALIAS(HH\\heapgraph_foreach_edge, heapgraph_foreach_edge);
    HHVM_FALIAS(HH\\heapgraph_foreach_root, heapgraph_foreach_root);
    HHVM_FALIAS(HH\\heapgraph_edge, heapgraph_edge);
    HHVM_FALIAS(HH\\heapgraph_node, heapgraph_node);
    HHVM_FALIAS(HH\\heapgraph_node_out_edges, heapgraph_node_out_edges);
    HHVM_FALIAS(HH\\heapgraph_node_in_edges, heapgraph_node_in_edges);
    HHVM_FALIAS(HH\\heapgraph_dfs_nodes, heapgraph_dfs_nodes);
    HHVM_FALIAS(HH\\heapgraph_dfs_edges, heapgraph_dfs_edges);

    loadSystemlib();
  }
} s_heapgraph_extension;


///////////////////////////////////////////////////////////////////////////////
}
