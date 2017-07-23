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

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/memory-manager-defs.h"
#include "hphp/runtime/base/tv-refcount.h"
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
  std::unordered_set<const ObjectData*> sources;
};
struct ObjprofClassReferral {
  uint64_t refs{0};
  std::unordered_set<Class*> sources;
};

struct ObjprofMetrics {
  uint64_t instances{0};
  uint64_t bytes{0};
  double bytes_rel{0};
};
using PathsToObject = std::unordered_map<
  ObjectData*, std::unordered_map<std::string, ObjprofObjectReferral>>;
using PathsToClass = std::unordered_map<
  Class*, std::unordered_map<std::string, ObjprofClassReferral>>;

struct ObjprofStringAgg {
  uint64_t dups{0};
  uint64_t refs{0};
  uint64_t srefs{0};
  String path;
};

using ObjprofStrings = std::unordered_map<
  String,
  ObjprofStringAgg,
  hphp_string_hash,
  hphp_string_same
>;
using ObjprofStack = std::vector<std::string>;
using ClassProp = std::pair<Class*, std::string>;

// Stack of pointers used for avoiding back-links when performing a DFS scan
// starting at a root node.
using ObjprofValuePtrStack = std::vector<const void*>;

enum ObjprofFlags {
  DEFAULT = 1,
  USER_TYPES_ONLY = 2,
  PER_PROPERTY = 4
};

std::pair<int, double> tvGetSize(
  TypedValue tv,
  int ref_adjust,
  const ObjectData* source,
  ObjprofStack* stack,
  PathsToObject* paths,
  ObjprofValuePtrStack* val_stack,
  const std::unordered_set<std::string>& exclude_classes,
  ObjprofFlags flags
);
void tvGetStrings(
  TypedValue tv,
  ObjprofStrings* metrics,
  ObjprofStack* path,
  std::unordered_set<void*>* pointers);

std::pair<int, double> getObjSize(
  const ObjectData* obj,
  const ObjectData* source,
  ObjprofStack* stack,
  PathsToObject* paths,
  ObjprofValuePtrStack* val_stack,
  const std::unordered_set<std::string>& exclude_classes,
  std::unordered_map<ClassProp, ObjprofMetrics>* histogram,
  ObjprofFlags flags
);

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
 * Determines whether the given object is a "root" node. Class instances
 * are considered root nodes, with the exception of classes starting with
 * "HH\\" (collections, wait handles etc)
 */
bool isObjprofRoot(
  const ObjectData* obj,
  ObjprofFlags flags,
  const std::unordered_set<std::string>& exclude_classes
) {
  Class* cls = obj->getVMClass();
  auto cls_name = cls->name()->toCppString();
  // Classes in exclude_classes not considered root
  if (exclude_classes.find(cls_name) != exclude_classes.end()) return false;
  // In USER_TYPES_ONLY mode, Classes with "HH\\" prefix not considered root
  if ((flags & ObjprofFlags::USER_TYPES_ONLY) != 0) {
    if (cls_name.compare(0, 3, "HH\\") == 0) return false;
  }
  return true;
}

/**
 * Measures the size of the array and referenced objects without going
 * into ObjectData* references.
 *
 * These are not measured:
 * kApcKind     // APCArray
 * kGlobalsKind // GlobalsArray
 * kProxyKind   // ProxyArray
 */
std::pair<int, double> sizeOfArray(
  const ArrayData* ad,
  const ObjectData* source,
  ObjprofStack* stack,
  PathsToObject* paths,
  ObjprofValuePtrStack* val_stack,
  const std::unordered_set<std::string>& exclude_classes,
  Class* cls,
  std::unordered_map<ClassProp, ObjprofMetrics>* histogram,
  ObjprofFlags flags
) {
  auto arrKind = ad->kind();
  if (
    arrKind == ArrayData::ArrayKind::kApcKind ||
    arrKind == ArrayData::ArrayKind::kGlobalsKind ||
    arrKind == ArrayData::ArrayKind::kProxyKind
  ) {
    return std::make_pair(0, 0);
  }

  auto ptr_begin = val_stack->begin();
  auto ptr_end = val_stack->end();
  if (std::find(ptr_begin, ptr_end, ad) != ptr_end) {
    FTRACE(3, "Cycle found for ArrayData*({})\n", ad);
    return std::make_pair(0, 0);
  }
  FTRACE(3, "\n\nInserting ArrayData*({})\n", ad);
  val_stack->push_back(ad);

  int size = 0;
  double sized = 0;
  if (ad->hasPackedLayout()) {
    FTRACE(2, "Iterating packed array\n");
    if (stack) stack->push_back("ArrayIndex");

    IterateV(ad, [&] (TypedValue v) {
      auto val_size_pair = tvGetSize(
        v,
        0, /* ref_adjust */
        source,
        stack,
        paths,
        val_stack,
        exclude_classes,
        flags
      );
      if (histogram) {
        auto histogram_key = std::make_pair(cls, "<index>");
        auto& metrics = (*histogram)[histogram_key];
        metrics.instances += 1;
        metrics.bytes += val_size_pair.first;
        metrics.bytes_rel += val_size_pair.second;
      }
      size += val_size_pair.first;
      sized += val_size_pair.second;
      FTRACE(2, "Value size for item was {}\n", val_size_pair.first);
      return false;
    });

    if (stack) stack->pop_back();
  } else {
    FTRACE(2, "Iterating mixed array\n");
    IterateKV(ad, [&] (Cell k, TypedValue v) {

      std::pair<int, double> key_size_pair;
      switch (k.m_type) {
        case KindOfPersistentString:
        case KindOfString: {
          auto const str = k.m_data.pstr;
          if (stack) {
            stack->push_back(
              std::string("ArrayKeyString:" + str->toCppString()));
          }
          key_size_pair = tvGetSize(
            k,
            0, /* ref_adjust */
            source,
            stack,
            paths,
            val_stack,
            exclude_classes,
            flags
          );
          FTRACE(2, "  Iterating str-key {} with size {}:{}\n",
            str->data(), key_size_pair.first, key_size_pair.second);
          break;
        }
        case KindOfInt64: {
          int64_t num = k.m_data.num;
          if (stack) {
            stack->push_back(std::string("ArrayKeyInt:" + std::to_string(num)));
          }
          key_size_pair = tvGetSize(
            k,
            0, /* ref_adjust */
            source,
            stack,
            paths,
            val_stack,
            exclude_classes,
            flags
          );
          FTRACE(2, "  Iterating num-key {} with size {}:{}\n",
            num, key_size_pair.first, key_size_pair.second);
          break;
        }
        case KindOfUninit:
        case KindOfNull:
        case KindOfPersistentVec:
        case KindOfBoolean:
        case KindOfPersistentDict:
        case KindOfDouble:
        case KindOfPersistentArray:
        case KindOfPersistentKeyset:
        case KindOfObject:
        case KindOfResource:
        case KindOfVec:
        case KindOfDict:
        case KindOfRef:
        case KindOfArray:
        case KindOfKeyset:
          always_assert(false);
      }

      auto val_size_pair = tvGetSize(
        v,
        0, /* ref_adjust */
        source,
        stack,
        paths,
        val_stack,
        exclude_classes,
        flags
      );
      FTRACE(2, "  Value size for that key was {}:{}\n",
        val_size_pair.first, val_size_pair.second);
      if (histogram) {
        auto histogram_key = std::make_pair(
          cls,
          String::attach(tvCastToString(k)).c_str()
        );
        auto& metrics = (*histogram)[histogram_key];
        metrics.instances += 1;
        metrics.bytes += val_size_pair.first + key_size_pair.first;
        metrics.bytes_rel += val_size_pair.second + key_size_pair.second;
      }
      size += val_size_pair.first + key_size_pair.first;
      sized += val_size_pair.second + key_size_pair.second;

      if (stack) stack->pop_back();
      return false;
    });
  }

  FTRACE(3, "Popping {} frm stack in sizeOfArray. Stack size before pop {}\n",
    val_stack->back(), val_stack->size()
  );
  val_stack->pop_back();

  return std::make_pair(size, sized);
}

void stringsOfArray(
  const ArrayData* ad,
  ObjprofStrings* metrics,
  ObjprofStack* path,
  std::unordered_set<void*>* pointers
) {
  path->push_back(std::string("array()"));

  if (ad->hasPackedLayout()) {
    path->push_back(std::string("[]"));
    IterateV(ad, [&] (TypedValue v) {
      tvGetStrings(v, metrics, path, pointers);
      return false;
    });
    path->pop_back();
  } else {
    IterateKV(ad, [&] (Cell k, TypedValue v) {
      switch (k.m_type) {
        case KindOfPersistentString:
        case KindOfString: {
          auto const str = k.m_data.pstr;
          tvGetStrings(k, metrics, path, pointers);
          path->push_back(std::string("[\"" + str->toCppString() + "\"]"));
          break;
        }
        case KindOfInt64: {
          auto const num = k.m_data.num;
          auto key_str = std::to_string(num);
          path->push_back(std::string(key_str));
          tvGetStrings(k, metrics, path, pointers);
          path->pop_back();
          path->push_back(std::string("[" + key_str + "]"));
          break;
        }
        case KindOfUninit:
        case KindOfNull:
        case KindOfPersistentVec:
        case KindOfBoolean:
        case KindOfPersistentDict:
        case KindOfDouble:
        case KindOfPersistentArray:
        case KindOfPersistentKeyset:
        case KindOfObject:
        case KindOfResource:
        case KindOfVec:
        case KindOfDict:
        case KindOfRef:
        case KindOfArray:
        case KindOfKeyset:
          // this should be an always_assert(false), but that appears to trigger
          // a gcc-4.9 bug (t16350411); even after t16350411 is fixed, we
          // can't always_assert(false) here until we stop supporting gcc-4.9
          // for open source users, since they may be using an older version
          assert(false);
      }

      tvGetStrings(v, metrics, path, pointers);
      path->pop_back();
      return false;
    });
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
 */
std::pair<int, double> tvGetSize(
  TypedValue tv,
  int ref_adjust,
  const ObjectData* source,
  ObjprofStack* stack,
  PathsToObject* paths,
  ObjprofValuePtrStack* val_stack,
  const std::unordered_set<std::string>& exclude_classes,
  ObjprofFlags flags
) {
  int size = sizeof(tv);
  double sized = size;

  switch (tv.m_type) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble: {
      // Counted as part sizeof(TypedValue)
      break;
    }
    case KindOfObject: {
      ObjectData* obj = tv.m_data.pobj;
      // If its not a root node, recurse into the object to determine its size
      if (!isObjprofRoot(obj, flags, exclude_classes)) {
        auto obj_size_pair = getObjSize(
          obj,
          source,
          stack,
          paths,
          val_stack,
          exclude_classes,
          nullptr, /* histogram */
          flags
        );
        size += obj_size_pair.first;
        if (obj->isRefCounted()) {
          auto obj_ref_count = tvGetCount(tv) + ref_adjust;
          FTRACE(3, " ObjectData tv: at {} with ref count {} after adjust {}\n",
            (void*)obj,
            obj_ref_count,
            ref_adjust
          );
          if (obj_ref_count > 0) {
            sized += obj_size_pair.second / (double)(obj_ref_count);
          }
        }
      } else if (stack && paths) {
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
      break;
    }
    case KindOfPersistentVec:
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentArray:
    case KindOfArray: {
      ArrayData* arr = tv.m_data.parr;
      auto size_of_array_pair = sizeOfArray(
        arr,
        source,
        stack,
        paths,
        val_stack,
        exclude_classes,
        nullptr, /* cls */
        nullptr, /* histogram */
        flags
      );
      if (arr->isRefCounted()) {
        auto arr_ref_count = tvGetCount(tv) + ref_adjust;
        FTRACE(3, " ArrayData tv: at {} with ref count {} after adjust {}\n",
          (void*)arr,
          arr_ref_count,
          ref_adjust
        );
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
        size += sizeof(*arr);
        size += size_of_array_pair.first;
      }

      break;
    }
    case KindOfResource: {
      auto res_ref_count = tvGetCount(tv) + ref_adjust;
      auto resource = tv.m_data.pres;
      auto resource_size = resource->heapSize();
      size += resource_size;
      if (res_ref_count > 0) {
        sized += resource_size / (double)(res_ref_count);
      }
      break;
    }
    case KindOfRef: {
      RefData* ref = tv.m_data.pref;
      size += sizeof(*ref);
      sized += sizeof(*ref);

      auto ref_ref_count = ref->getRealCount() + ref_adjust;
      FTRACE(3, " RefData tv at {} that with ref count {} after adjust {}\n",
        (void*)ref,
        ref_ref_count,
        ref_adjust
      );

      Cell* cell = ref->tv();
      auto size_of_tv_pair = tvGetSize(
        *cell,
        0, /* ref_adjust */
        source,
        stack,
        paths,
        val_stack,
        exclude_classes,
        flags
      );
      size += size_of_tv_pair.first;

      if (ref_ref_count > 0) {
        sized += size_of_tv_pair.second / (double)(ref_ref_count);
      }
      break;
    }
    case KindOfPersistentString:
    case KindOfString: {
      StringData* str = tv.m_data.pstr;
      size += str->size();
      if (str->isRefCounted()) {
        auto str_ref_count = tvGetCount(tv) + ref_adjust;
        FTRACE(3, " String tv: {} string at {} ref count: {} after adjust {}\n",
          str->data(),
          (void*)str,
          str_ref_count,
          ref_adjust
        );
        if (str_ref_count > 0) {
          sized += (str->size() / (double)(str_ref_count));
        }
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
  TypedValue tv,
  ObjprofStrings* metrics,
  ObjprofStack* path,
  std::unordered_set<void*>* pointers
) {
  switch (tv.m_type) {
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
    case HPHP::KindOfPersistentVec:
    case HPHP::KindOfVec:
    case HPHP::KindOfPersistentDict:
    case HPHP::KindOfDict:
    case HPHP::KindOfPersistentKeyset:
    case HPHP::KindOfKeyset:
    case HPHP::KindOfPersistentArray:
    case HPHP::KindOfArray: {
      ArrayData* arr = tv.m_data.parr;
      stringsOfArray(arr, metrics, path, pointers);
      break;
    }
    case HPHP::KindOfRef: {
      RefData* ref = tv.m_data.pref;
      Cell* cell = ref->tv();
      tvGetStrings(*cell, metrics, path, pointers);
      break;
    }
    case HPHP::KindOfPersistentString:
    case HPHP::KindOfString: {
      StringData* str = tv.m_data.pstr;

      // Obtain aggregation object
      auto &str_agg = (*metrics)[StrNR(str)];
      if (!str_agg.path.get()) {
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

      FTRACE(3, " String: {} = {} \n",
        pathString(path, ":").get()->data(),
        str->data()
      );
      break;
    }
  }
}

bool supportsToArray(const ObjectData* obj) {
  if (obj->isCollection()) {
    // we never want to toArray on a collection; if we're asking if we can,
    // then something has gone horribly wrong
    always_assert(false);
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
  const ObjectData* obj,
  const ObjectData* source,
  ObjprofStack* stack,
  PathsToObject* paths,
  ObjprofValuePtrStack* val_stack,
  const std::unordered_set<std::string>& exclude_classes,
  std::unordered_map<ClassProp, ObjprofMetrics>* histogram,
  ObjprofFlags flags
) {
  Class* cls = obj->getVMClass();

  auto ptr_begin = val_stack->begin();
  auto ptr_end = val_stack->end();
  if (std::find(ptr_begin, ptr_end, obj) != ptr_end) {
    FTRACE(3, "Cycle found for {}*({})\n", obj->getClassName().data(), obj);
    return std::make_pair(0, 0);
  }
  FTRACE(3, "\n\nInserting {}*({})\n", obj->getClassName().data(), obj);
  val_stack->push_back(obj);

  FTRACE(1, "Getting object size for type {} at {}\n",
    obj->getClassName().data(),
    obj
  );
  int size;
  if (UNLIKELY(obj->getAttribute(ObjectData::IsWaitHandle))) {
    size = asio_object_size(obj);
  } else {
    size = sizeof(ObjectData);
  }
  double sized = size;

  if (stack) stack->push_back(
    std::string("Object:" + cls->name()->toCppString())
  );

  if (obj->isCollection()) {
    auto const arr = collections::asArray(obj);
    if (arr) {
      auto array_size_pair = sizeOfArray(
        arr,
        source,
        stack,
        paths,
        val_stack,
        exclude_classes,
        cls,
        histogram,
        flags
      );
      size += array_size_pair.first;
      sized += array_size_pair.second;
    } else {
      assertx(collections::isType(cls, CollectionType::Pair));
      auto pair = static_cast<const c_Pair*>(obj);
      auto elm_size_pair = tvGetSize(
        *pair->get(0),
        0, /* ref_adjust */
        source,
        stack,
        paths,
        val_stack,
        exclude_classes,
        flags
      );
      size += elm_size_pair.first;
      sized += elm_size_pair.second;
      elm_size_pair = tvGetSize(
        *pair->get(1),
        0, /* ref_adjust */
        source,
        stack,
        paths,
        val_stack,
        exclude_classes,
        flags
      );
      size += elm_size_pair.first;
      sized += elm_size_pair.second;
    }

    if (stack) stack->pop_back();
    return std::make_pair(size, sized);
  }

  if (!supportsToArray(obj)) {
    if (stack) stack->pop_back();
    return std::make_pair(size, sized);
  }

  // We're increasing ref count by calling toArray, need to adjust it later
  auto arr = obj->toArray(); // TODO t12985984 avoid toArray.
  bool is_packed = arr->hasPackedLayout();

  for (ArrayIter iter(arr); iter; ++iter) {
    auto const key_tv = *iter.first().asTypedValue();
    auto const val_tv = iter.secondVal();
    auto key = tvAsCVarRef(&key_tv).toString();
    if (isStringType(key_tv.m_type)) {
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

    bool is_declared = isStringType(key_tv.m_type) &&
                       cls->lookupDeclProp(key.get()) != kInvalidSlot;

    int key_size = 0;
    double key_sized = 0;
    if (!is_declared && !is_packed) {
      FTRACE(2, "Counting string key {} because it's non-declared/packed\n",
        key.c_str());
      auto key_size_pair = tvGetSize(
        key_tv,
        -1, /* ref_adjust */
        source,
        stack,
        paths,
        val_stack,
        exclude_classes,
        flags
      );
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
    auto val_size_pair = tvGetSize(
      val_tv,
      -1, /* ref_adjust */
      source,
      stack,
      paths,
      val_stack,
      exclude_classes,
      flags
    );

    FTRACE(2, "   Summary for key {} with size key={}:{}, val={}:{}\n",
      key.c_str(),
      key_size,
      key_sized,
      val_size_pair.first,
      val_size_pair.second
    );

    if (histogram) {
      auto histogram_key = std::make_pair(
        cls,
        is_packed ? "<index>" : std::string(key.c_str())
      );
      auto& metrics = (*histogram)[histogram_key];
      metrics.instances += 1;
      metrics.bytes += val_size_pair.first + key_size;
      metrics.bytes_rel += val_size_pair.second + key_sized;
    }

    size += val_size_pair.first + key_size;
    sized += val_size_pair.second + key_sized;
    if (stack) stack->pop_back();
  }
  if (stack) stack->pop_back();

  FTRACE(3, "Popping {} frm stack in getObjSize. Stack size before pop {}\n",
    val_stack->back(), val_stack->size()
  );
  val_stack->pop_back();

  return std::make_pair(size, sized);
}

void getObjStrings(
  const ObjectData* obj,
  ObjprofStrings* metrics,
  ObjprofStack* path,
  std::unordered_set<void*>* pointers
) {
  Class* cls = obj->getVMClass();
  FTRACE(1, "Getting strings for type {} at {}\n",
    obj->getClassName().data(),
    obj
  );

  if (obj->isCollection()) {
    auto const arr = collections::asArray(obj);
    path->push_back(obj->getClassName().data());
    if (arr) {
      stringsOfArray(arr, metrics, path, pointers);
    } else {
      assertx(collections::isType(cls, CollectionType::Pair));
      auto pair = static_cast<const c_Pair*>(obj);
      tvGetStrings(*pair->get(0), metrics, path, pointers);
      tvGetStrings(*pair->get(1), metrics, path, pointers);
    }
    path->pop_back();
    return;
  }

  if (!supportsToArray(obj)) {
    return;
  }

  path->push_back(obj->getClassName().data());
  auto arr = obj->toArray(); // TODO t12985984 avoid toArray.
  bool is_packed = arr->hasPackedLayout();

  for (ArrayIter iter(arr); iter; ++iter) {
    auto first = iter.first();
    auto key = first.toString();
    auto key_tv = *first.asTypedValue();
    auto val_tv = iter.secondVal();

    if (isStringType(key_tv.m_type)) {
      // If the key begins with a NUL, it's a private or protected property.
      // Read the class name from between the two NUL bytes.
      //
      // Note: Copied from object-data.cpp
      if (!key.empty() && key[0] == '\0') {
        int subLen = key.find('\0', 1) + 1;
        key = key.substr(subLen);
      }
    }

    bool is_declared = isStringType(key_tv.m_type) &&
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

  std::unordered_set<void*> pointers;
  MM().forEachObject([&](const ObjectData* obj) {
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

Array HHVM_FUNCTION(objprof_get_data,
  int flags = ObjprofFlags::DEFAULT,
  const Array& exclude_list = Array()
) {
  std::unordered_map<ClassProp, ObjprofMetrics> histogram;
  auto objprof_props_mode = (flags & ObjprofFlags::PER_PROPERTY) != 0;

  // Create a set of std::strings from the exclude_list provided. This de-dups
  // the exclude_list, and also provides for fast lookup when determining
  // whether a given class is in the exclude_list
  std::unordered_set<std::string> exclude_classes;
  for (ArrayIter iter(exclude_list); iter; ++iter) {
    exclude_classes.insert(iter.second().toString().data());
  }

  MM().forEachObject([&](const ObjectData* obj) {
    if (!isObjprofRoot(obj, (ObjprofFlags)flags, exclude_classes)) return;
    std::vector<const void*> val_stack;
    auto objsizePair = getObjSize(
      obj,
      nullptr, /* source */
      nullptr, /* stack */
      nullptr, /* paths */
      &val_stack,
      exclude_classes,
      objprof_props_mode ? &histogram : nullptr,
      (ObjprofFlags)flags
    );

    if (!objprof_props_mode) {
      auto cls = obj->getVMClass();
      auto cls_name = cls->name()->toCppString();
      auto& metrics = histogram[std::make_pair(cls, "")];
      metrics.instances += 1;
      metrics.bytes += objsizePair.first;
      metrics.bytes_rel += objsizePair.second;

      FTRACE(1, "ObjectData* at {} ({}) size={}:{}\n",
       obj,
       cls_name,
       objsizePair.first,
       objsizePair.second
      );
    }
  });

  // Create response
  ArrayInit objs(histogram.size(), ArrayInit::Map{});
  for (auto const& it : histogram) {
    auto c = it.first;
    auto cls = c.first;
    auto prop = c.second;
    auto key = cls->name()->toCppString();
    if (prop != "") {
      key += "::" + c.second;
    }

    auto metrics_val = make_map_array(
      s_instances, Variant(it.second.instances),
      s_bytes, Variant(it.second.bytes),
      s_bytes_rel, it.second.bytes_rel,
      s_paths, init_null()
    );

    objs.set(StrNR(key), Variant(metrics_val));
  }

  return objs.toArray();
}

Array HHVM_FUNCTION(objprof_get_paths,
  int flags = ObjprofFlags::DEFAULT,
  const Array& exclude_list = Array()
) {
  std::unordered_map<ClassProp, ObjprofMetrics> histogram;
  PathsToClass pathsToClass;

  // Create a set of std::strings from the exclude_list provided. This de-dups
  // the exclude_list, and also provides for fast lookup when determining
  // whether a given class is in the exclude_list
  std::unordered_set<std::string> exclude_classes;
  for (ArrayIter iter(exclude_list); iter; ++iter) {
    exclude_classes.insert(iter.second().toString().data());
  }

  MM().forEachObject([&](const ObjectData* obj) {
      if (!isObjprofRoot(obj, (ObjprofFlags)flags, exclude_classes)) return;
      auto cls = obj->getVMClass();
      auto& metrics = histogram[std::make_pair(cls, "")];
      ObjprofStack stack;
      PathsToObject pathsToObject;
      std::vector<const void*> val_stack;
      auto objsizePair = getObjSize(
        obj, /* obj */
        obj, /* source */
        &stack,
        &pathsToObject,
        &val_stack,
        exclude_classes,
        nullptr, /* histogram */
        (ObjprofFlags)flags
      );
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
    std::vector<const void*> val_stack;
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
      tvGetSize(
        *tv,
        -1, /* ref_adjust */
        nullptr, /* source */
        &stack,
        &pathsToObject,
        &val_stack,
        exclude_classes,
        (ObjprofFlags)flags
      );
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
    auto clsPaths = pathsToClass[c.first];
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

    objs.set(c.first->nameStr(), Variant(metrics_val));
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

struct objprofExtension final : Extension {
  objprofExtension() : Extension("objprof", "1.0") { }

  void moduleInit() override {
    HHVM_FALIAS(HH\\objprof_get_data, objprof_get_data);
    HHVM_FALIAS(HH\\objprof_get_strings, objprof_get_strings);
    HHVM_FALIAS(HH\\objprof_get_paths, objprof_get_paths);
    HHVM_FALIAS(HH\\thread_memory_stats, thread_memory_stats);
    HHVM_FALIAS(HH\\thread_mark_stack, thread_mark_stack);
    HHVM_FALIAS(HH\\set_mem_threshold_callback, set_mem_threshold_callback);
    HHVM_RC_INT(OBJPROF_FLAGS_DEFAULT, ObjprofFlags::DEFAULT);
    HHVM_RC_INT(OBJPROF_FLAGS_USER_TYPES_ONLY, ObjprofFlags::USER_TYPES_ONLY);
    HHVM_RC_INT(OBJPROF_FLAGS_PER_PROPERTY, ObjprofFlags::PER_PROPERTY);
    loadSystemlib();
  }
} s_objprof_extension;


///////////////////////////////////////////////////////////////////////////////
}
