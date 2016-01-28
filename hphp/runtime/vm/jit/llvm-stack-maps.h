/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_RUNTIME_VM_JIT_LLVM_STACK_MAPS_H_
#define incl_HPHP_RUNTIME_VM_JIT_LLVM_STACK_MAPS_H_

#include "hphp/runtime/vm/jit/containers.h"

#include <cstdint>

namespace HPHP { namespace jit {

/*
 * Our representation of the data parsed out from LLVM stack maps:
 * http://llvm.org/docs/StackMaps.html
 */
struct StackMaps {
  uint8_t version;

  struct StkSizeRecord {
    uint8_t* funcAddr;
    uint64_t stkSize;
  };
  jit::vector<StkSizeRecord> stkSizeRecords;

  jit::vector<uint64_t> constants;

  struct Record {
    uint64_t id;
    uint32_t offset;

    struct Location {
      enum class Kind : uint8_t {
        Register   = 0x1,
        Direct     = 0x2,
        Indirect   = 0x3,
        Constant   = 0x4,
        ConstIndex = 0x5,
      };

      Kind kind;
      uint16_t regNum;
      int32_t offsetOrConst;
    };
    jit::vector<Location> locations;

    struct LiveOut {
      uint16_t regNum;
      uint8_t size;
    };
    jit::vector<LiveOut> liveOuts;
  };

  jit::flat_map<uint64_t, Record> records;
};

/*
 * Parse the stack map section from LLVM codegen into a StackMaps struct.
 */
StackMaps parseStackMaps(const uint8_t* ptr);

/*
 * Pretty-print a StackMaps struct.
 */
std::string show(const StackMaps& maps);

} }

#endif
