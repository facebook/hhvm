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

#include <cstdint>
#include <optional>
#include <vector>

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/jit/cprof-record.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP {

struct StringData;

namespace jit {

struct ProfDataTargetProfile;

namespace cprof {

/* Process-local target profile ready to install under a fresh TransID. */
struct ContProfPreparedTargetProfile {
  ContProfTargetProfileKind kind{ContProfTargetProfileKind::DecRef};
  Offset bytecodeOffset{0};
  const StringData* name{nullptr};
  std::vector<uint8_t> payload;
};

/* Snapshot a supported live RDS target profile. */
std::optional<ContProfTargetProfile>
snapshotContProfTargetProfile(
  const rds::Profile& profile,
  rds::Handle handle,
  uint32_t allocationSize
);

/* Validate and intern a target profile for this process. */
std::optional<ContProfPreparedTargetProfile>
prepareContProfTargetProfile(const ContProfTargetProfile& profile);

/*
 * Install a prepared target profile under a newly allocated TransID.
 * Each profile key must be installed only once.
 */
bool installContProfTargetProfile(
  const ContProfPreparedTargetProfile& profile,
  TransID transId,
  ProfDataTargetProfile& targetProfiles
);

}
}

}
