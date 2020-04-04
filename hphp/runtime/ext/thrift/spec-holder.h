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

#ifndef incl_HPHP_EXT_SPEC_HOLDER_H_
#define incl_HPHP_EXT_SPEC_HOLDER_H_

#include "hphp/runtime/ext/thrift/ext_thrift.h"
#include "hphp/runtime/ext/thrift/transport.h"
#include "hphp/runtime/ext/thrift/util.h"

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/util/fixed-vector.h"

#include <folly/AtomicHashMap.h>

namespace HPHP { namespace thrift {

Array get_tspec(const Class* cls);

struct FieldSpec {
  int16_t fieldNum;
  TType type;
  StringData* name;  // TODO(9396341): Consider using LowStringPtr.
  ArrayData* spec;
  bool isUnion;
  bool noTypeCheck; // If this field doesn't need type checking
                    // (conservatively).
};

using StructSpec = FixedVector<FieldSpec>;

using SpecCacheKey = std::tuple<const ArrayData*, const Class*, bool>;

struct SpecCacheCompare {
  bool equal(const SpecCacheKey& a, const SpecCacheKey& b) const {
    return a == b;
  }
  size_t hash(const SpecCacheKey& k) const {
    return
      folly::hash::hash_combine(
        folly::hash::hash_combine(
          pointer_hash<ArrayData>{}(std::get<0>(k)),
          pointer_hash<Class>{}(std::get<1>(k))
        ),
        std::get<2>(k)
      );
  }
};

using SpecCacheMap = tbb::concurrent_hash_map<
  SpecCacheKey,
  StructSpec*,
  SpecCacheCompare
>;

// Provides safe access to specifications.
struct SpecHolder {
  // The returned reference is valid at least while this SpecHolder is alive.
  const StructSpec& getSpec(const Array& spec,
                            const Object& obj,
                            bool isBinary) {
    // Since noTypeCheck depends on the enclosing object class and what type of
    // serialization we're doing, we need to include that in the key. If we're
    // not verify property type-hints we can skip this (and get more potential
    // sharing). If the class isn't persistent, we can't elide any type checks
    // anyways, so set it to null (which makes it pessimistic).
    auto cls = obj->getVMClass();
    if (RuntimeOption::EvalCheckPropTypeHints <= 0 ||
        !classHasPersistentRDS(cls)) {
      cls = nullptr;
      isBinary = false;
    }

    SpecCacheKey key{ spec.get(), cls, isBinary };

    {
      SpecCacheMap::const_accessor acc;
      if (s_specCacheMap.find(acc, key)) return *acc->second;
    }

    if (spec->isStatic()) {
      // Static specs are kept by the cache.
      SpecCacheMap::accessor acc;
      if (s_specCacheMap.insert(acc, key)) {
        acc->second = new StructSpec{compileSpec(spec, cls, isBinary)};
      }
      return *acc->second;
    } else {
      // Temporary specs are kept by m_tempSpec.
      StructSpec temp(compileSpec(spec, nullptr, false));
      m_tempSpec.swap(temp);
      return m_tempSpec;
    }
  }

 private:
  // Non-static spec, or empty if source spec is static.
  StructSpec m_tempSpec;
  static SpecCacheMap s_specCacheMap;

  // Check if the field-spec implies that the field's type-constraint will
  // always be satisfied. We don't need to do type verification if so.
  static bool typeSatisfiesConstraint(const TypeConstraint& tc,
                                      TType type,
                                      const Array& fieldSpec,
                                      bool isBinary);

  static StructSpec compileSpec(const Array& spec,
                                const Class* cls,
                                bool isBinary) {
    std::vector<FieldSpec> temp(spec.size());
    ArrayIter specIt = spec.begin();
    for (int i = 0; i < spec.size(); ++i, ++specIt) {
      if (!specIt.first().isInteger()) {
        thrift_error("Bad keytype in TSPEC (expected 'long')",
                     ERR_INVALID_DATA);
      }
      auto& field = temp[i];
      field.fieldNum = specIt.first().toInt16();
      Array fieldSpec = specIt.second().toArray();
      field.spec = fieldSpec.get();
      field.type = (TType)tvCastToInt64(
        fieldSpec.rval(s_type, AccessFlags::ErrorKey).tv()
      );
      field.name = tvCastToStringData(
        fieldSpec.rval(s_var, AccessFlags::ErrorKey).tv()
      );
      field.isUnion = tvCastToBoolean(
        fieldSpec.rval(s_union, AccessFlags::Key).tv()
      );

      // A union field also writes to a property named __type. If one exists, we
      // need to also verify that it accepts integer values. We only need to do
      // this once, so cache it in the optional.
      folly::Optional<bool> endPropOk;

      // Determine if we can safely skip the type check when deserializing.
      field.noTypeCheck = [&] {
        // Check this first, so we skip the type check even if cls is null.
        if (RuntimeOption::EvalCheckPropTypeHints <= 0) return true;
        if (!cls) return false;
        auto const slot = cls->lookupDeclProp(field.name);
        if (slot == kInvalidSlot) return false;

        if (field.isUnion) {
          if (!endPropOk) {
            endPropOk = [&] {
              if (cls->numDeclProperties() < spec.size()) return false;
              auto const& prop = cls->declProperties()[spec.size()];
              if (!s__type.equal(prop.name)) return false;
              return prop.typeConstraint.alwaysPasses(KindOfInt64);
            }();
          }
          if (!*endPropOk) return false;
        }

        return typeSatisfiesConstraint(
          cls->declPropTypeConstraint(slot),
          field.type,
          fieldSpec,
          isBinary
        );
      }();
    }

    if (temp.size() >> 16) {
      thrift_error("Too many keys in TSPEC (expected < 2^16)",
                   ERR_INVALID_DATA);
    }
    return StructSpec(temp);
  }
};

}}

#endif
