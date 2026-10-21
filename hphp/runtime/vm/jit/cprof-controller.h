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

#include <cstddef>
#include <string>
#include <vector>

#include "hphp/runtime/vm/jit/cprof-record.h"

namespace HPHP {

struct Func;

}

namespace HPHP::jit {

struct ProfData;

}

namespace HPHP::jit::cprof {

/*
 * Capture at a sync or async profile-optimization boundary, if configured and
 * active. `async` also records the async-only subset of opportunities.
 * Startup replay deliberately does not call this hook.
 */
void maybeCaptureContProfProfile(const ProfData&, const Func&, bool async = false);

/*
 * Snapshot and retain the first valid profile record for `func`.
 * Replace a retained replay record, or insert a new record. Later captures of
 * the same function are duplicates. Returns true iff the stored record changed.
 * Does not check capture eligibility (the optimization hook does that).
 */
bool captureContProfProfile(const ProfData&, const Func&);

/* Retain a compatible replay record unchanged, unless its key is already stored. */
bool retainContProfProfileRecord(const ContProfProfileRecord&);

size_t numContProfProfileRecords();

/* Best-effort loaded repo-unit paths, sampled after the profile records. */
std::vector<std::string> snapshotContProfLoadedUnits();

/*
 * Finalize pending target profiles and return a key-ordered snapshot of the
 * captured records. The caller must be in a treadmill session.
 */
std::vector<ContProfProfileRecord> snapshotContProfProfileRecords();

}
