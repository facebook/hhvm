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

#include "hphp/runtime/ext/extension.h"

/*
 *                       Project ObjProf
 * What is it?
 * Breakdown of allocated memory by object types.
 *
 * How does it work?
 * We traverse all instances of ObjectData* to measure their memory.
 *
 * How do I use it?
 * Call objprof_get_data to trigger the scan.
 */

#include <set>
#include <unordered_map>
#include <vector>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/memory-manager-defs.h"
#include "hphp/runtime/ext/datetime/ext_datetime.h"
#include "hphp/runtime/ext/simplexml/ext_simplexml.h"
#include "hphp/runtime/ext/std/ext_std_closure.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/named-entity-defs.h"
#include "hphp/util/alloc.h"

namespace HPHP {
size_t asio_object_size(const ObjectData*);

namespace {

TRACE_SET_MOD(objprof);

///////////////////////////////////////////////////////////////////////////////

const StaticString
  s_cpp_stack("cpp_stack"),
  s_cpp_stack_peak("cpp_stack_peak"),
  s_dups("dups"),
  s_refs("refs"),
  s_srefs("srefs"),
  s_path("path"),
  s_paths("paths"),
  s_bytes("bytes"),
  s_bytes_rel("bytes_normalized"),
  s_instances("instances");

struct ObjprofObjectReferral {
  uint64_t refs{0};
  std::unordered_set<ObjectData*> sources;
};
struct ObjprofClassReferral {
  uint64_t refs{0};
  std::unordered_set<Class*> sources;
};


struct ObjprofMetrics {
public:
  uint64_t instances{0};
  uint64_t bytes{0};
  double bytes_rel{0};
};
using PathsToObject = std::unordered_map<
  ObjectData*, std::unordered_map<std::string, ObjprofObjectReferral>>;
using PathsToClass = std::unordered_map<
  Class*, std::unordered_map<std::string, ObjprofClassReferral>>;

struct ObjprofStringAgg {
public:
  uint64_t dups;
  uint64_t refs;
  uint64_t srefs;
  String path;
};

using ObjprofStrings = std::unordered_map<
  StringData*,
  ObjprofStringAgg,
  string_data_hash,
  string_data_same
>;
using ObjprofStack = std::vector<std::string>;

std::pair<int, double> tvGetSize(
  const TypedValue* tv,
  int ref_adjust,
  ObjectData* source,
  ObjprofStack* stack,
  PathsToObject* paths,
  int depthAllowed
);
void tvGetStrings(
  const TypedValue* tv,
  ObjprofStrings* metrics,
  ObjprofStack* path,
  std::set<void*>* pointers);

String pathString(ObjprofStack* stack, const char* sep) {
  assert(stack->size() < 100000000);
  StringBuffer sb;
  for (size_t i = 0; i < stack->size(); ++i) {
    if (i != 0) sb.append(sep);
    sb.append((*stack)[i]);
  }
  return sb.detach();
}

/**
 * Measures the size of the array and referenced objects without going
 * into ObjectData* references.
 *
 * These are not measured:
 * kEmptyKind   // The singleton static empty array
 * kSharedKind  // SharedArray
 * kGlobalsKind // GlobalsArray
 * kProxyKind   // ProxyArray
 * kNumKinds    // insert new values before kNumKinds.
 */
std::pair<int, double> sizeOfArray(
  const ArrayData* props,
  ObjectData* source,
  ObjprofStack* stack,
  PathsToObject* paths
) {
  auto arrKind = props->kind();
  if (
    arrKind != ArrayData::ArrayKind::kPackedKind &&
    arrKind != ArrayData::ArrayKind::kStructKind &&
    arrKind != ArrayData::ArrayKind::kMixedKind &&
    arrKind != ArrayData::ArrayKind::kEmptyKind
  ) {
    return std::make_pair(0, 0);
  }

  ssize_t iter = props->iter_begin();
  auto pos_limit = props->iter_end();
  int size = 0;
  double sized = 0;

  auto handle_dense_array_item = [&] () {
    const TypedValue* val = props->getValueRef(iter).asTypedValue();
    auto val_size_pair = tvGetSize(val, 0, source, stack, paths, 0);
    size += val_size_pair.first;
    sized += val_size_pair.second;
    FTRACE(2, "Value size for item was {}\n", val_size_pair.first);
  };

  if (props->isMixed()) {
    FTRACE(2, "Iterating mixed array\n");
    while (iter != pos_limit) {
      // Get key
      TypedValue key;
      MixedArray::NvGetKey(props, &key, iter);
      // Measure val
      const TypedValue* val;
      std::pair<int, double> key_size_pair;
      switch (key.m_type) {
        case KindOfString: {
          StringData* str = key.m_data.pstr;
          val = MixedArray::NvGetStr(props, str);
          if (stack) {
            auto key_str = str->toCppString();
            stack->push_back(std::string("ArrayKeyString:" + key_str));
          }
          key_size_pair = tvGetSize(&key, -1, source, stack, paths, 0);
          FTRACE(2, "  Iterating str-key {} with size {}:{}\n",
            str->data(),
            key_size_pair.first,
            key_size_pair.second
          );
          str->decRefCount();
          break;
        }
        case KindOfInt64: {
          int64_t num = key.m_data.num;
          val = MixedArray::NvGetInt(props, num);
          if (stack) {
            auto key_str = std::to_string(num);
            stack->push_back(std::string("ArrayKeyInt:" + key_str));
          }
          key_size_pair = tvGetSize(&key, 0, source, stack, paths, 0);
          FTRACE(2, "  Iterating num-key {} with size {}:{}\n",
            num,
            key_size_pair.first,
            key_size_pair.second
          );
          break;
        }
        default:
          always_assert(false);
      }

      auto val_size_pair = tvGetSize(val, 0, source, stack, paths, 0);
      FTRACE(2, "  Value size for that key was {}:{}\n",
        val_size_pair.first,
        val_size_pair.second
      );
      size += val_size_pair.first + key_size_pair.first;
      sized += val_size_pair.second + key_size_pair.second;
      iter = MixedArray::IterAdvance(props, iter);
      if (stack) stack->pop_back();
    }
  } else if (props->isPacked()) {
    FTRACE(2, "Iterating packed array\n");
    while (iter != pos_limit) {
      if (stack) stack->push_back("ArrayIndex");
      handle_dense_array_item();
      iter = PackedArray::IterAdvance(props, iter);
      if (stack) stack->pop_back();
    }
  } else if (props->isStruct()) {
    FTRACE(2, "Iterating struct array\n");
    while (iter != pos_limit) {
      if (stack) stack->push_back("StructIndex");
      handle_dense_array_item();
      iter = StructArray::IterAdvance(props, iter);
      if (stack) stack->pop_back();
    }
  }

  return std::make_pair(size, sized);
}

void stringsOfArray(
  const ArrayData* props,
  ObjprofStrings* metrics,
  ObjprofStack* path,
  std::set<void*>* pointers
) {
  ssize_t iter = props->iter_begin();
  auto pos_limit = props->iter_end();
  path->push_back(std::string("array()"));

  auto handle_dense_array_item = [&]() {
    const TypedValue* val = props->getValueRef(iter).asTypedValue();
    tvGetStrings(val, metrics, path, pointers);
  };

  if (props->isMixed()) {
    while (iter != pos_limit) {
      // Get key
      TypedValue key;
      MixedArray::NvGetKey(props, &key, iter);
      // Measure val
      const TypedValue* val;
      switch (key.m_type) {
        case KindOfString: {
          StringData* str = key.m_data.pstr;
          val = MixedArray::NvGetStr(props, str);
          auto key_str = str->toCppString();
          str->decRefCount();
          tvGetStrings(&key, metrics, path, pointers);
          path->push_back(std::string("[\"" + key_str + "\"]"));
          break;
        }
        case KindOfInt64: {
          int64_t num = key.m_data.num;
          val = MixedArray::NvGetInt(props, num);
          auto key_str = std::to_string(num);
          path->push_back(std::string(key_str));
          tvGetStrings(&key, metrics, path, pointers);
          path->pop_back();
          path->push_back(std::string("[" + key_str + "]"));
         break;
        }
        default:
          always_assert(false);
      }

      tvGetStrings(val, metrics, path, pointers);
      path->pop_back();
      iter = MixedArray::IterAdvance(props, iter);
    }
  } else if (props->isPacked()) {
    path->push_back(std::string("[]"));
    while (iter != pos_limit) {
      handle_dense_array_item();
      iter = PackedArray::IterAdvance(props, iter);
    }
    path->pop_back();
  } else if (props->isStruct()) {
    path->push_back(std::string("[]"));
    while (iter != pos_limit) {
      handle_dense_array_item();
      iter = StructArray::IterAdvance(props, iter);
    }
    path->pop_back();
  }

  path->pop_back();
}

/**
 * Measures the size of the typed value and referenced objects without going
 * into ObjectData* references.
 *
 * These are not measured:
 * kEmptyKind   // The singleton static empty array
 * kSharedKind  // SharedArray
 * kGlobalsKind // GlobalsArray
 * kProxyKind   // ProxyArray
 * kNumKinds    // insert new values before kNumKinds.
 */
std::pair<int, double> tvGetSize(
  const TypedValue* tv,
  int ref_adjust,
  ObjectData* source,
  ObjprofStack* stack,
  PathsToObject* paths,
  int depthAllowed
) {
  int size = sizeof(*tv);
  double sized = size;

  switch (tv->m_type) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfClass: {
      // Counted as part sizeof(TypedValue)
      break;
    }
    case KindOfObject: {
      if (stack && paths) {
        ObjectData* obj = tv->m_data.pobj;
        // notice we might have multiple OBJ->path->OBJ for same path
        // (e.g. packed array where we omit the index number)
        auto cls = obj->getVMClass();
        stack->push_back(std::string("Object:" + cls->name()->toCppString()));
        auto pathStr = pathString(stack, "->");
        stack->pop_back();

        auto& pathsToTv = (*paths)[obj];
        auto& referral = pathsToTv[pathStr.toCppString()];
        if (source) {
          referral.sources.insert(source);
        }
        referral.refs += 1;

        FTRACE(3, " ObjectData tv: at {} of type {} at path {}, refs {}\n",
          (void*)obj,
          obj->getClassName().data(),
          pathStr.data(),
          referral.refs
        );
      }
      // This is a shallow size function, not a recursive one
      break;
    }
    case KindOfPersistentArray:
    case KindOfArray: {
      ArrayData* arr = tv->m_data.parr;
      if (arr->isRefCounted()) {
        auto arr_ref_count = tvGetCount(tv) + ref_adjust;
        FTRACE(3, " ArrayData tv: at {} with ref count {} after adjust {}\n",
          (void*)arr,
          arr_ref_count,
          ref_adjust
        );
        auto size_of_array_pair = sizeOfArray(arr, source, stack, paths);
        size += sizeof(*arr);
        size += size_of_array_pair.first;
        if (arr_ref_count > 0) {
          sized += sizeof(*arr) / (double)arr_ref_count;
          sized += size_of_array_pair.second / (double)(arr_ref_count);
        }
      } else {
        // static or uncounted array
        FTRACE(3, " ArrayData tv: at {} not refcounted, after adjust {}\n",
          (void*)arr, ref_adjust
        );
        auto size_of_array_pair = sizeOfArray(arr, source, stack, paths);
        size += sizeof(*arr);
        size += size_of_array_pair.first;
      }
      break;
    }
    case KindOfResource: {
      auto res_ref_count = tvGetCount(tv) + ref_adjust;
      auto resource = tv->m_data.pres;
      auto resource_size = resource->heapSize();
      size += resource_size;
      if (res_ref_count > 0) {
        sized += resource_size / (double)(res_ref_count);
      }
      break;
    }
    case KindOfRef: {
      RefData* ref = tv->m_data.pref;
      size += sizeof(*ref);
      sized += sizeof(*ref);

      auto ref_ref_count = ref->getRealCount() + ref_adjust;
      FTRACE(3, " RefData tv at {} that with ref count {} after adjust {}\n",
        (void*)ref,
        ref_ref_count,
        ref_adjust
      );

      Cell* cell = ref->tv();
      auto size_of_tv_pair =
        tvGetSize((TypedValue*)cell, 0, source, stack, paths, 0);
      size += size_of_tv_pair.first;

      if (ref_ref_count > 0) {
        sized += size_of_tv_pair.second / (double)(ref_ref_count);
      }
      break;
    }
    case KindOfPersistentString:
    case KindOfString: {
      StringData* str = tv->m_data.pstr;
      size += str->size();
      if (str->isRefCounted()) {
        auto str_ref_count = tvGetCount(tv) + ref_adjust;
        FTRACE(3, " String tv: {} string at {} ref count: {} after adjust {}\n",
          str->data(),
          (void*)str,
          str_ref_count,
          ref_adjust
        );
        sized += (str->size() / (double)(str_ref_count));
      } else {
        // static or uncounted string
        FTRACE(3, " String tv: {} string at {} uncounted, after adjust {}\n",
          str->data(), (void*)str, ref_adjust
        );
      }
      break;
    }
  }

  return std::make_pair(size, sized);
}

void tvGetStrings(
  const TypedValue* tv,
  ObjprofStrings* metrics,
  ObjprofStack* path,
  std::set<void*>* pointers
) {
  switch (tv->m_type) {
    case HPHP::KindOfUninit:
    case HPHP::KindOfResource:
    case HPHP::KindOfNull:
    case HPHP::KindOfBoolean:
    case HPHP::KindOfInt64:
    case HPHP::KindOfDouble: {
      // Not strings
      break;
    }
    case HPHP::KindOfObject: {
      // This is a shallow size function, not a recursive one
      break;
    }
    case HPHP::KindOfArray: {
      ArrayData* arr = tv->m_data.parr;
      stringsOfArray(arr, metrics, path, pointers);
      break;
    }
    case HPHP::KindOfRef: {
      RefData* ref = tv->m_data.pref;
      Cell* cell = ref->tv();
      tvGetStrings((TypedValue*)cell, metrics, path, pointers);
      break;
    }
    case HPHP::KindOfPersistentString:
    case HPHP::KindOfString: {
      StringData* str = tv->m_data.pstr;

      // Obtain aggregation object
      auto metrics_it = metrics->find(str);
      ObjprofStringAgg str_agg;
      if (metrics_it != metrics->end()) {
        str_agg = metrics_it->second;
      } else {
        str_agg.dups = 0;
        str_agg.refs = 0;
        str_agg.srefs = 0;
        str_agg.path = pathString(path, ":");
      }

      // Increment aggregation metrics
      str_agg.refs++;
      if (pointers->find(str) == pointers->end()) {
        pointers->insert(str);
        str_agg.dups++;
      }
      if (!str->isRefCounted()) {
        str_agg.srefs++;
      }

      (*metrics)[str] = str_agg;
      FTRACE(3, " String: {} = {} \n",
        pathString(path, ":").get()->data(),
        str->data()
      );
      break;
    }
    default:
      // Not interesting
      break;
  }
}

int getClassSize(Class* cls) {
  int size = 0;
  auto precls = cls->preClass();
  size += precls->builtinObjSize();
  size += sizeof(ObjectData);
  return size;
}

bool supportsToArray(ObjectData* obj) {
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

std::pair<int, double> getObjSize(
  ObjectData* obj,
  ObjectData* source,
  ObjprofStack* stack,
  PathsToObject* paths
) {
  Class* cls = obj->getVMClass();
  FTRACE(1, "Getting object size for type {} at {}\n",
    obj->getClassName().data(),
    obj
  );
  int size;
  if (UNLIKELY(obj->getAttribute(ObjectData::IsWaitHandle))) {
    size = asio_object_size(obj);
  } else {
    size = getClassSize(cls);
  }

  double sized = size;
  if (stack) stack->push_back(
    std::string("Object:" + cls->name()->toCppString())
  );

  if (!supportsToArray(obj)) {
    if (stack) stack->pop_back();
    return std::make_pair(size, sized);
  }

  bool adjust_val = true;
  if (collections::isType(cls, CollectionType::Map, CollectionType::ImmMap)) {
    adjust_val = false;
  }

  // We're increasing ref count by calling toArray, need to adjust it later
  auto arr = obj->toArray();
  bool is_packed = arr->isPacked();

  for (ArrayIter iter(arr); iter; ++iter) {
    TypedValue key_tv = *iter.first().asTypedValue();
    auto val_tv = iter.secondRef().asTypedValue();
    auto key = tvAsVariant(&key_tv).toString();
    if (key_tv.m_type == HPHP::KindOfString) {
      // If the key begins with a NUL, it's a private or protected property.
      // Read the class name from between the two NUL bytes.
      //
      // Note: Copied from object-data.cpp
      if (!key.empty() && key[0] == '\0') {
        int subLen = key.find('\0', 1) + 1;
        key = key.substr(subLen);
        FTRACE(3, "Resolved private prop name: {}\n", key.c_str());
      }
    }

    bool is_declared =
        key_tv.m_type == HPHP::KindOfString &&
        cls->lookupDeclProp(key.get()) != kInvalidSlot;

    int key_size = 0;
    double key_sized = 0;
    if (!is_declared && !is_packed) {
      FTRACE(2, "Counting string key {} because it's non-declared/packed\n",
        key.c_str());
      auto key_size_pair = tvGetSize(&key_tv, -1, source, stack, paths, 0);
      key_size = key_size_pair.first;
      key_sized = key_size_pair.second;
      if (stack) stack->push_back(std::string("Key:"+key));
    } else {
      FTRACE(2, "Skipping key {} because it's declared/packed\n", key.c_str());
      if (is_packed) {
        if (stack) stack->push_back(std::string("PropertyIndex"));
      } else {
        if (stack) stack->push_back(std::string("Property:" + key));
      }
    }

    FTRACE(2, "Counting value for key {}\n", key.c_str());
    auto val_size_pair =
      tvGetSize(val_tv, adjust_val ? -1 : 0, source, stack, paths, 0);
    FTRACE(2, "   Summary for key {} with size key={}:{}, val={}:{}\n",
      key.c_str(),
      key_size,
      key_sized,
      val_size_pair.first,
      val_size_pair.second
    );

    size += val_size_pair.first + key_size;
    sized += val_size_pair.second + key_sized;
    if (stack) stack->pop_back();
  }
  if (stack) stack->pop_back();
  return std::make_pair(size, sized);
}

void getObjStrings(
  ObjectData* obj,
  ObjprofStrings* metrics,
  ObjprofStack* path,
  std::set<void*>* pointers
) {
  Class* cls = obj->getVMClass();
  FTRACE(1, "Getting strings for type {} at {}\n",
    obj->getClassName().data(),
    obj
  );

  if (!supportsToArray(obj)) {
    return;
  }

  path->push_back(std::string(cls->name()->data()));
  auto arr = obj->toArray();
  bool is_packed = arr->isPacked();

  for (ArrayIter iter(arr); iter; ++iter) {
    auto first = iter.first();
    auto key = first.toString();
    auto key_tv = first.asTypedValue();
    auto val_tv = iter.secondRef().asTypedValue();

    if (key_tv->m_type == HPHP::KindOfString) {
      // If the key begins with a NUL, it's a private or protected property.
      // Read the class name from between the two NUL bytes.
      //
      // Note: Copied from object-data.cpp
      if (!key.empty() && key[0] == '\0') {
        int subLen = key.find('\0', 1) + 1;
        key = key.substr(subLen);
      }
    }

    bool is_declared =
        key_tv->m_type == HPHP::KindOfString &&
        cls->lookupDeclProp(key.get()) != kInvalidSlot;

    if (!is_declared && !is_packed) {
      FTRACE(2, "Inspecting key {} because it's non-declared/packed\n",
        key.c_str());
      tvGetStrings(key_tv, metrics, path, pointers);
      path->push_back(std::string("[\"" + key + "\"]"));
    } else {
      FTRACE(2, "Skipping key {} because it's declared/packed\n", key.c_str());
      path->push_back(std::string(key));
    }

    FTRACE(2, "Inspecting value for key {}\n", key.c_str());
    tvGetStrings(val_tv, metrics, path, pointers);
    path->pop_back();
    FTRACE(2, "   Finished for key/val {}\n",
      key.c_str()
    );
  }
  path->pop_back();
}


///////////////////////////////////////////////////////////////////////////////
// Function that traverses objects and counts metrics per strings

Array HHVM_FUNCTION(objprof_get_strings, int min_dup) {
  ObjprofStrings metrics;

  std::set<void*> pointers;
  MM().forEachObject([&](ObjectData* obj) {
      ObjprofStack path;
      getObjStrings(obj, &metrics, &path, &pointers);
  });

  // Create response
  ArrayInit objs(metrics.size(), ArrayInit::Map{});
  for (auto& it : metrics) {
    if (it.second.dups < min_dup) continue;

    auto metrics_val = make_map_array(
      s_dups, Variant(it.second.dups),
      s_refs, Variant(it.second.refs),
      s_srefs, Variant(it.second.srefs),
      s_path, Variant(it.second.path)
    );

    const Variant str = Variant(it.first);
    objs.setValidKey(str, Variant(metrics_val));
  }

  return objs.toArray();
}


///////////////////////////////////////////////////////////////////////////////
// Function that inits the scan of the memory and count of class pointers

Array HHVM_FUNCTION(objprof_get_data, void) {
  std::unordered_map<Class*,ObjprofMetrics> histogram;
  MM().forEachObject([&](ObjectData* obj) {
      auto cls = obj->getVMClass();
      auto objsizePair = getObjSize(obj, nullptr, nullptr, nullptr);
      auto& metrics = histogram[cls];
      metrics.instances += 1;
      metrics.bytes += objsizePair.first;
      metrics.bytes_rel += objsizePair.second;

      FTRACE(1, "ObjectData* at {} ({}) size={}:{}\n",
             obj,
             obj->getClassName().data(),
             objsizePair.first,
             objsizePair.second
            );
  });

  // Create response
  ArrayInit objs(histogram.size(), ArrayInit::Map{});
  for (auto const& it : histogram) {
    auto c = it.first;

    auto metrics_val = make_map_array(
      s_instances, Variant(it.second.instances),
      s_bytes, Variant(it.second.bytes),
      s_bytes_rel, it.second.bytes_rel,
      s_paths, init_null()
    );

    objs.set(c->nameStr(), Variant(metrics_val));
  }

  return objs.toArray();
}

Array HHVM_FUNCTION(objprof_get_paths, void) {
  std::unordered_map<Class*, ObjprofMetrics> histogram;
  PathsToClass pathsToClass;

  MM().forEachObject([&](ObjectData* obj) {
      auto cls = obj->getVMClass();
      auto& metrics = histogram[cls];
      ObjprofStack stack;
      PathsToObject pathsToObject;
      auto objsizePair = getObjSize(obj, obj, &stack, &pathsToObject);
      metrics.instances += 1;
      metrics.bytes += objsizePair.first;
      metrics.bytes_rel += objsizePair.second;
      for (auto const& pathsIt : pathsToObject) {
        auto cls = pathsIt.first->getVMClass();
        auto& paths = pathsIt.second;
        auto& aggPaths = pathsToClass[cls];
        for (auto const& pathKV : paths) {
          auto& path = pathKV.first;
          auto& referral = pathKV.second;
          auto& aggReferral = aggPaths[path];
          aggReferral.refs += referral.refs;
          aggReferral.sources.insert(obj->getVMClass());
        }
      }

      FTRACE(1, "ObjectData* at {} ({}) size={}:{}\n",
       obj,
       obj->getClassName().data(),
       objsizePair.first,
       objsizePair.second
      );
      assert(stack.size() == 0);
  });

  NamedEntity::foreach_class([&](Class* cls) {
    if (cls->needsInitSProps()) {
      return;
    }
    auto const staticProps = cls->staticProperties();
    auto const nSProps = cls->numStaticProperties();
    for (Slot i = 0; i < nSProps; ++i) {
      auto const& prop = staticProps[i];
      auto tv = cls->getSPropData(i);
      if (tv == nullptr) {
        continue;
      }

      FTRACE(2, "Traversing static prop {}::{}\n",
        cls->name()->data(),
        StrNR(prop.name).data()
      );

      ObjprofStack stack;
      PathsToObject pathsToObject;

      auto refname = std::string(
        "ClassProperty:" +
        cls->name()->toCppString() + ":" +
        StrNR(prop.name).data()
      );

      if (tv->m_data.num == 0) {
        continue;
      }

      stack.push_back(refname);
      tvGetSize(tv, -1, nullptr, &stack, &pathsToObject, 0);
      stack.pop_back();

      for (auto const& pathsIt : pathsToObject) {
        auto cls = pathsIt.first->getVMClass();
        auto& paths = pathsIt.second;
        auto& aggPaths = pathsToClass[cls];
        for (auto const& pathKV : paths) {
          auto& path = pathKV.first;
          auto& referral = pathKV.second;
          auto& aggReferral = aggPaths[path];
          aggReferral.refs += referral.refs;
          aggReferral.sources.insert(cls);
        }
      }
      assert(stack.size() == 0);
    }
  });

  // Create response
  ArrayInit objs(histogram.size(), ArrayInit::Map{});
  for (auto const& it : histogram) {
    auto c = it.first;
    auto clsPaths = pathsToClass[c];
    ArrayInit pathsArr(clsPaths.size(), ArrayInit::Map{});
    for (auto const& pathIt : clsPaths) {
      auto pathStr = pathIt.first;
      auto path_metrics_val = make_map_array(
        s_refs, pathIt.second.refs
      );

      pathsArr.setValidKey(Variant(pathStr), Variant(path_metrics_val));
    }

    auto metrics_val = make_map_array(
      s_instances, Variant(it.second.instances),
      s_bytes, Variant(it.second.bytes),
      s_bytes_rel, it.second.bytes_rel,
      s_paths, Variant(pathsArr.toArray())
    );

    objs.set(c->nameStr(), Variant(metrics_val));
  }

  return objs.toArray();
}

///////////////////////////////////////////////////////////////////////////////

size_t get_thread_stack_size() {
  auto sp = stack_top_ptr();
  return s_stackLimit + s_stackSize - uintptr_t(sp);
}

size_t get_thread_stack_peak_size() {
  size_t consecutive = 0;
  size_t total = 0;
  uint8_t marker = 0x00;
  uint8_t* cursor = &marker;
  uintptr_t cursor_p = uintptr_t(&marker);
  for (;cursor_p > s_stackLimit; cursor_p--, cursor--) {
    total++;
    if (*cursor == 0x00) {
      if (++consecutive == s_pageSize) {
        return get_thread_stack_size() + total - consecutive;
      }
    } else {
      consecutive = 0;
    }
  }

  return s_stackSize;
}

void HHVM_FUNCTION(thread_mark_stack, void) {
  size_t consecutive = 0;
  uint8_t marker = 0x00;
  uint8_t* cursor = &marker;
  uintptr_t cursor_p = uintptr_t(&marker);
  for (;cursor_p > s_stackLimit; cursor_p--, cursor--) {
    if (*cursor == 0x00) {
      if (++consecutive == s_pageSize) {
        return;
      }
    } else {
      consecutive = 0;
      *cursor = 0x00;
    }
  }
}

Array HHVM_FUNCTION(thread_memory_stats, void) {
  auto stack_size = get_thread_stack_size();
  auto stack_size_peak = get_thread_stack_peak_size();

  auto stats = make_map_array(
      s_cpp_stack, Variant(stack_size),
      s_cpp_stack_peak, Variant(stack_size_peak)
  );

  return stats;
}

///////////////////////////////////////////////////////////////////////////////

void HHVM_FUNCTION(set_mem_threshold_callback,
  int64_t threshold,
  const Variant& callback
) {
  // In a similar way to fb_setprofile storing in m_setprofileCallback
  g_context->m_memThresholdCallback = callback;

  // Notify MM that surprise flag should be set upon reaching the threshold
  MM().setMemThresholdCallback(threshold);
}

///////////////////////////////////////////////////////////////////////////////

}

class objprofExtension final : public Extension {
public:
  objprofExtension() : Extension("objprof", "1.0") { }

  void moduleInit() override {
    HHVM_FALIAS(HH\\objprof_get_data, objprof_get_data);
    HHVM_FALIAS(HH\\objprof_get_strings, objprof_get_strings);
    HHVM_FALIAS(HH\\objprof_get_paths, objprof_get_paths);
    HHVM_FALIAS(HH\\thread_memory_stats, thread_memory_stats);
    HHVM_FALIAS(HH\\thread_mark_stack, thread_mark_stack);
    HHVM_FALIAS(HH\\set_mem_threshold_callback, set_mem_threshold_callback);
    loadSystemlib();
  }
} s_objprof_extension;


///////////////////////////////////////////////////////////////////////////////
}
