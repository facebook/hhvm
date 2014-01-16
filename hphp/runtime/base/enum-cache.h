/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_ENUM_CACHE_H_
#define incl_HPHP_ENUM_CACHE_H_

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/vm/class.h"
#include <tbb/concurrent_hash_map.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////


class EnumCache {
public:
  EnumCache() {}
  ~EnumCache();

  // TBB hash and compare struct
  struct clsCompare {
    bool equal(intptr_t key1, intptr_t key2) const {
      assert(key1 && key2);
      bool equal = (key1 == key2);
      assert(!equal || getClass(key1)->name()->equal(getClass(key2)->name()));
      return equal;
    }

    size_t hash(intptr_t key) const {
      assert(key);
      return static_cast<size_t>(hash_int64(key));
    }
  };

  // Values in the TBB map that contain the enum static arrays
  struct EnumValues {
    // array from 'enum name' to 'enum value'
    // e.g. [ 'RED' => 1, 'BLUE' =>2, ...]
    Array values;
    // array from 'enum value' to 'enum name'
    // e.g. [ 1 => 'RED', 2 => 'BLUE', ...]
    Array names;
  };

  // if the class provided derives from Enum the name/value and value/name
  // arrays are build, stored in the cache and returned.
  // If not an error is raised.
  // If the recurse flag is 'true' array values are loaded up the hierarchy
  // chain (if any).
  static const EnumValues* getValues(const Class* klass, bool recurse);
  // delete the EnumValues element in the cache for the given class.
  // If there is no entry this function is a no-op.
  static void deleteValues(const Class* klass);

  // Helper that raises a PHP exception
  static void failLookup(CVarRef msg) ATTRIBUTE_NORETURN;

private:
  // Class* to intptr_ti key helpers
  const static intptr_t RECURSE_MASK = 1;
  static const Class* getClass(intptr_t key) {
    return reinterpret_cast<const Class*>(key & ~RECURSE_MASK);
  }

  static intptr_t getKey(const Class* klass, bool recurse) {
    intptr_t key = reinterpret_cast<intptr_t>(klass);
    return (recurse) ? key | RECURSE_MASK : key;
  }

  const EnumValues* getEnumValuesIfDefined(intptr_t key) const;
  const EnumValues* getEnumValues(const Class* klass, bool recurse);
  const EnumValues* loadEnumValues(const Class* klass, bool recurse);
  void deleteEnumValues(intptr_t key);

  // Map that contains associations between Enum classes and their array
  // values and array names.
  typedef tbb::concurrent_hash_map<
              intptr_t, const EnumValues*, clsCompare> EnumValuesMap;

  EnumValuesMap m_enumValuesMap;
};

//////////////////////////////////////////////////////////////////////

}

#endif

