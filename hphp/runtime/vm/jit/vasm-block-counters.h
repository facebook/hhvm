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

#include "hphp/runtime/vm/jit/prof-data-serialize.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

struct RegionDesc;

///////////////////////////////////////////////////////////////////////////////

namespace VasmBlockCounters {

/*
 * If we have profiling data for the given region, return the profiled weight
 * of its entry block. Otherwise, return std::nullopt.
 */
Optional<uint64_t> getRegionWeight(const RegionDesc& region);

/*
 * Profile-guided update the given Vunit. This is only applied to optimized
 * translations and when jumpstart is enabled.  In this case, if the profile
 * data will be serialized, then profile counters are inserted in the unit's
 * blocks.  And if JIT profile data was deserialized, then the unit's block
 * weights will be updated based on the profile data.
 */
void profileGuidedUpdate(Vunit& unit);

/*
 * Serialize/deserialize.
 */
void serialize(ProfDataSerializer& ser);

void deserialize(ProfDataDeserializer& des);

void free();

}

///////////////////////////////////////////////////////////////////////////////

} }
