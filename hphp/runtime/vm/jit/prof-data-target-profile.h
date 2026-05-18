/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include <memory>

#include "hphp/runtime/base/rds-symbol.h"
#include "hphp/util/assertions.h"
#include "hphp/util/hash-map.h"
#include "hphp/util/optional.h"

namespace HPHP {

struct StringData;

#define PR(T)         \
struct T;
  RDS_PROFILE_SYMBOLS
#undef PR

namespace jit {

struct ProfDataTargetProfile {

  ~ProfDataTargetProfile();

  template <typename T>
  const T* get(const rds::Profile& key) const;

  struct RdsProfileHasher {
    size_t operator()(const rds::Profile& key) const;
  };
  struct RdsProfileEquals {
    bool operator()(const rds::Profile& key1,
                    const rds::Profile& key2) const;
  };

#define PR(T)                                                  \
 private:                                                      \
  hphp_fast_map<const rds::Profile, T*, RdsProfileHasher, RdsProfileEquals> m_map ## T; \
                                                               \
 public:                                                       \
  void add(const rds::Profile& key, T* value) {                \
    m_map##T.emplace(key, value);                              \
  }
  RDS_PROFILE_SYMBOLS
#undef PR
};

#define PR(T)                                                  \
template <>                                                    \
inline const T* ProfDataTargetProfile::get<T>(const rds::Profile& key) const { \
  auto it = m_map##T.find(key);                                \
  if (it == m_map##T.end()) return nullptr;                    \
  return it->second;                                           \
}
RDS_PROFILE_SYMBOLS
#undef PR

}

}
