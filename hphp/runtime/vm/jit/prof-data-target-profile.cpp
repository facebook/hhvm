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

#include "hphp/runtime/vm/jit/prof-data-target-profile.h"

#include "hphp/runtime/base/string-data.h"

namespace HPHP::jit {

ProfDataTargetProfile::~ProfDataTargetProfile() {
#define PR(T)                                                    \
  for (auto it = m_map##T.begin(); it != m_map##T.end(); ++it) { \
    free(it->second);                                            \
  }
  RDS_PROFILE_SYMBOLS
#undef PR
}

size_t ProfDataTargetProfile::RdsProfileHasher::operator()(const rds::Profile& key) const {
  return folly::hash::hash_combine(
    key.kind,
    key.transId,
    key.bcOff,
    key.name->hash()
  );
}

bool ProfDataTargetProfile::RdsProfileEquals::operator()(const rds::Profile& key1,
                                  const rds::Profile& key2) const {
  return
    key1.kind == key2.kind &&
    key1.transId == key2.transId &&
    key1.bcOff == key2.bcOff &&
    key1.name->same(key2.name);
}

} // namespace HPHP::jit
