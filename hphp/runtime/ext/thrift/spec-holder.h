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

#ifndef incl_HPHP_EXT_SPEC_HOLDER_H_
#define incl_HPHP_EXT_SPEC_HOLDER_H_

#include "hphp/runtime/ext/thrift/transport.h"
#include "hphp/runtime/ext/thrift/ext_thrift.h"
#include "hphp/util/fixed-vector.h"

#include <folly/AtomicHashMap.h>

namespace HPHP { namespace thrift {

enum TError {
  ERR_UNKNOWN = 0,
  ERR_INVALID_DATA = 1,
  ERR_BAD_VERSION = 4
};

ATTRIBUTE_NORETURN void thrift_error(const String& what, TError why);

Array get_tspec(const Class* cls);

struct FieldSpec {
  int16_t fieldNum;
  TType type;
  StringData* name;  // TODO(9396341): Consider using LowStringPtr.
  ArrayData* spec;
};

using StructSpec = FixedVector<FieldSpec>;
using SpecCacheMap = folly::AtomicHashMap<const ArrayData*, StructSpec>;

// Provides safe access to specifications.
class SpecHolder {
 public:
  // The returned reference is valid at least while this SpecHolder is alive.
  const StructSpec& getSpec(const Array& spec) {
    auto it = s_specCacheMap.find(spec.get());
    if (it != s_specCacheMap.end()) {
      return it->second;
    } else {
      if (spec->isStatic()) {
        // Static specs are kept by the cache.
        auto result = s_specCacheMap.insert(spec.get(), compileSpec(spec));
        // If someone else inserted the same key since our call to 'find', the
        // temporary StructSpec instance above will be destructed, and the
        // following will return the exisitng value from the cache:
        return result.first->second;
      } else {
        // Temporary specs are kept by m_tempSpec.
        StructSpec temp(compileSpec(spec));
        m_tempSpec.swap(temp);
        return m_tempSpec;
      }
    }
  }

 private:
  // Non-static spec, or empty if source spec is static.
  StructSpec m_tempSpec;
  static SpecCacheMap s_specCacheMap;

  static StructSpec compileSpec(const Array& spec) {
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
      field.type =
        (TType)fieldSpec.rvalAt(s_type, AccessFlags::Error_Key).toInt64();
      field.name =
        fieldSpec.rvalAt(s_var, AccessFlags::Error_Key).toString().get();
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
