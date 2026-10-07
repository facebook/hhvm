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
 * Snapshot and retain the first valid profile record for `func`.
 * Returns true iff a new record was inserted.
 */
bool captureContProfProfile(const ProfData&, const Func&);

size_t numContProfProfileRecords();

/*
 * Finalize pending target profiles and return a key-ordered snapshot of the
 * captured records. The caller must be in a treadmill session.
 */
std::vector<ContProfProfileRecord> snapshotContProfProfileRecords();

}
