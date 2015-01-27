/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_RUNTIME_VM_JIT_LLVM_LOCRECS_H_
#define incl_HPHP_RUNTIME_VM_JIT_LLVM_LOCRECS_H_

#include "hphp/runtime/vm/jit/containers.h"

#include <cstdint>
#include <unordered_map>

namespace HPHP { namespace jit {

/*
 * Representation of experimental LLVM location records.
 */
struct LocRecs {
  uint8_t versionMajor;
  uint8_t versionMinor;

  uint32_t numRecords;

  struct LocationRecord {
    uint8_t* address;
    uint32_t id;
    uint8_t  size;
  };

  // Maps ID to records
  jit::hash_map<uint32_t, jit::vector<LocationRecord>> records;
};

/*
 * Parse the location records section from LLVM codegen.
 */
LocRecs parseLocRecs(const uint8_t* ptr, size_t size);

/*
 * Pretty-print a LocRecs struct.
 */
std::string show(const LocRecs& recs);

} }

#endif
