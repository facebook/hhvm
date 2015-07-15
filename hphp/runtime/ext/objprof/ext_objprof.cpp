/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/unit.h"

TRACE_SET_MOD(objprof);

namespace HPHP {

const StaticString
  s_dups("dups"),
  s_refs("refs"),
  s_srefs("srefs"),
  s_path("path"),
  s_bytes("bytes"),
  s_bytes_rel("bytes_normalized"),
  s_instances("instances");

struct ObjprofMetrics {
public:
  uint64_t instances{0};
  uint64_t bytes{0};
  double bytes_rel{0};
};

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

std::pair<int, double> tvGetSize(const TypedValue* tv, int ref_adjust);
void tvGetStrings(
  const TypedValue* tv,
  ObjprofStrings* metrics,
  ObjprofStack* path,
  std::set<void*>* pointers);

String pathString(ObjprofStack* stack) {
  StringBuffer sb;
  for (size_t i = 0; i < stack->size(); ++i) {
    if (i != 0) sb.append(":");
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
std::pair<int, double> sizeOfArray(const ArrayData* props) {
  ssize_t iter = props->iter_begin();
  auto pos_limit = props->iter_end();
  int size = 0;
  double sized = 0;

  auto handle_dense_array_item = [&] () {
    const TypedValue* val = props->getValueRef(iter).asTypedValue();
    auto val_size_pair = tvGetSize(val, 0);
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
        case HPHP::KindOfString: {
          StringData* str = key.m_data.pstr;
          val = MixedArray::NvGetStr(props, str);
          key_size_pair = tvGetSize(&key, -1);
          FTRACE(2, "  Iterating str-key {} with size {}:{}\n",
            str->data(),
            key_size_pair.first,
            key_size_pair.second
          );
          str->decRefCount();
          break;
        }
        case HPHP::KindOfInt64: {
          int64_t num = key.m_data.num;
          val = MixedArray::NvGetInt(props, num);
          key_size_pair = tvGetSize(&key, 0);
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

      auto val_size_pair = tvGetSize(val, 0);
      FTRACE(2, "  Value size for that key was {}:{}\n",
        val_size_pair.first,
        val_size_pair.second
      );
      size += val_size_pair.first + key_size_pair.first;
      sized += val_size_pair.second + key_size_pair.second;
      iter = MixedArray::IterAdvance(props, iter);
    }
  } else if (props->isPacked()) {
    FTRACE(2, "Iterating packed array\n");
    while (iter != pos_limit) {
      handle_dense_array_item();
      iter = PackedArray::IterAdvance(props, iter);
    }
  } else if (props->isStruct()) {
    FTRACE(2, "Iterating struct array\n");
    while (iter != pos_limit) {
      handle_dense_array_item();
      iter = StructArray::IterAdvance(props, iter);
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
        case HPHP::KindOfString: {
          StringData* str = key.m_data.pstr;
          val = MixedArray::NvGetStr(props, str);
          auto key_str = str->toCppString();
          str->decRefCount();
          tvGetStrings(&key, metrics, path, pointers);
          path->push_back(std::string("[\"" + key_str + "\"]"));
          break;
        }
        case HPHP::KindOfInt64: {
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
std::pair<int, double> tvGetSize(const TypedValue* tv, int ref_adjust) {
  int size = sizeof(*tv);
  double sized = size;

  switch (tv->m_type) {
    case HPHP::KindOfUninit:
    case HPHP::KindOfNull:
    case HPHP::KindOfBoolean:
    case HPHP::KindOfInt64:
    case HPHP::KindOfDouble: {
      // Counted as part sizeof(TypedValue)
      break;
    }
    case HPHP::KindOfObject: {
      // This is a shallow size function, not a recursive one
      break;
    }
    case HPHP::KindOfArray: {
      ArrayData* arr = tv->m_data.parr;
      auto arr_ref_count = arr->getCount() + ref_adjust;
      FTRACE(3, " ArrayData tv: at {} that with ref count {} after adjust {}\n",
        (void*)arr,
        arr_ref_count,
        ref_adjust
      );
      auto size_of_array_pair = sizeOfArray(arr);
      size += sizeof(*arr);
      size += size_of_array_pair.first;
      if (arr_ref_count > 0) {
        sized += sizeof(*arr) / (double)arr_ref_count;
        sized += size_of_array_pair.second / (double)(arr_ref_count);
      }
      break;
    }
    case HPHP::KindOfResource: {
      // Not really counting the resource itself
      ResourceData* resource = tv->m_data.pres;
      auto res_ref_count = resource->getCount() + ref_adjust;
      size += sizeof(*resource);
      if (res_ref_count > 0) {
        sized += sizeof(*resource) / (double)(res_ref_count);
      }
      break;
    }
    case HPHP::KindOfRef: {
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
      auto size_of_tv_pair = tvGetSize((TypedValue*)cell, 0);
      size += size_of_tv_pair.first;

      if (ref_ref_count > 0) {
        sized += size_of_tv_pair.second / (double)(ref_ref_count);
      }
      break;
    }
    case HPHP::KindOfStaticString:
    case HPHP::KindOfString: {
      StringData* str = tv->m_data.pstr;
      size += str->size();
      auto str_ref_count = str->getCount() + ref_adjust;
      FTRACE(3, " String tv: {} string at {} ref count: {} after adjust {}\n",
        str->data(),
        (void*)str,
        str_ref_count,
        ref_adjust
      );

      if (str_ref_count > 0) {
        sized += (str->size() / (double)(str_ref_count));
      }
      break;
    }
    default:
      // Not interesting
      break;
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
    case HPHP::KindOfStaticString:
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
        str_agg.path = pathString(path);
      }

      // Increment aggregation metrics
      str_agg.refs++;
      if (pointers->find(str) == pointers->end()) {
        pointers->insert(str);
        str_agg.dups++;
      }
      if (str->getCount() < 0) {
        str_agg.srefs++;
      }

      (*metrics)[str] = str_agg;
      FTRACE(3, " String: {} = {} \n",
        pathString(path).get()->data(),
        str->data()
      );
      break;
    }
    default:
      // Not interesting
      break;
  }
}

static int getClassSize(Class* cls) {
  int size = 0;
  auto precls = cls->preClass();
  size += precls->builtinObjSize();
  size += sizeof(ObjectData);
  return size;
}

static bool supportsToArray(ObjectData* obj) {
  if (obj->isCollection()) {
    assertx(isValidCollection(obj->collectionType()));
    return true;
  } else if (UNLIKELY(obj->getAttribute(ObjectData::CallToImpl))) {
    return obj->instanceof(c_SimpleXMLElement::classof());
  } else if (UNLIKELY(obj->instanceof(SystemLib::s_ArrayObjectClass))) {
    return true;
  } else if (UNLIKELY(obj->instanceof(SystemLib::s_ArrayIteratorClass))) {
    return true;
  } else if (UNLIKELY(obj->instanceof(SystemLib::s_ClosureClass))) {
    return true;
  } else if (UNLIKELY(obj->instanceof(DateTimeData::getClass()))) {
    return true;
  } else {
    if (LIKELY(!obj->getAttribute(ObjectData::InstanceDtor))) {
      return true;
    }

    return false;
  }
}

static std::pair<int, double> getObjSize(ObjectData* obj) {
  Class* cls = obj->getVMClass();
  FTRACE(1, "Getting object size for type {} at {}\n",
    obj->getClassName().data(),
    obj
  );
  int size = getClassSize(cls);
  double sized = size;

  if (!supportsToArray(obj)) {
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
      auto key_size_pair = tvGetSize(&key_tv, -1);
      key_size = key_size_pair.first;
      key_sized = key_size_pair.second;
    } else {
      FTRACE(2, "Skipping key {} because it's declared/packed\n", key.c_str());
    }

    FTRACE(2, "Counting value for key {}\n", key.c_str());
    auto val_size_pair = tvGetSize(val_tv, adjust_val ? -1 : 0);
    FTRACE(2, "   Summary for key {} with size key={}:{}, val={}:{}\n",
      key.c_str(),
      key_size,
      key_sized,
      val_size_pair.first,
      val_size_pair.second
    );

   size += val_size_pair.first + key_size;
   sized += val_size_pair.second + key_sized;
  }
  return std::make_pair(size, sized);
}

static void getObjStrings(
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

static Array HHVM_FUNCTION(objprof_get_strings, int min_dup) {
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
    objs.set(str, Variant(metrics_val));
  }

  return objs.toArray();
}


///////////////////////////////////////////////////////////////////////////////
// Function that inits the scan of the memory and count of class pointers

static Array HHVM_FUNCTION(objprof_get_data, void) {
  std::unordered_map<Class*,ObjprofMetrics> histogram;
  MM().forEachObject([&](ObjectData* obj) {
      auto cls = obj->getVMClass();
      auto objsizePair = getObjSize(obj);
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
      s_bytes_rel, it.second.bytes_rel
    );

    objs.set(c->nameStr(), Variant(metrics_val));
  }

  return objs.toArray();
}

class objprofExtension final : public Extension {
 public:
  objprofExtension() : Extension("objprof", "1.0") { }

  void moduleInit() override {
    HHVM_FALIAS(HH\\objprof_get_data, objprof_get_data);
    HHVM_FALIAS(HH\\objprof_get_strings, objprof_get_strings);
    loadSystemlib();
  }
} s_objprof_extension;

///////////////////////////////////////////////////////////////////////////////

}
