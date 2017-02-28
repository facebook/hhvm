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
#include "hphp/tools/tc-print/mappers.h"

#include <cstdio>
#include <string>
#include <assert.h>
#include <algorithm>
#include <iomanip>

#include "hphp/runtime/vm/jit/translator.h"

#include "hphp/tools/tc-print/offline-code.h"
#include "hphp/tools/tc-print/tc-print.h"

namespace HPHP { namespace jit {

#define EXT_OPCODES \
  EXT_OP(TraceletGuard) \
  EXT_OP(FuncPrologue)

enum ExtOp {
  ExtOpStart = Op_count + 1,
#define EXT_OP(name) ExtOp##name ,
  EXT_OPCODES
#undef EXT_OP
  ExtOpEnd
};

/* Utilities */

std::string extOpcodeToString(ExtOpcode eOpcode) {
  static const char* extOpNames[] = {
#define EXT_OP(name) #name ,
    EXT_OPCODES
#undef EXT_OP
    "Invalid"
  };

  if (eOpcode < (ExtOpcode)ExtOpStart) {
    return opcodeToName((Op)eOpcode);
  }

  if (eOpcode > (ExtOpcode)ExtOpStart && eOpcode < (ExtOpcode)ExtOpEnd) {
    return extOpNames[eOpcode - ExtOpStart - 1];
  }

  return "Invalid";
}

std::vector<std::pair<std::string,ExtOpcode>> getValidOpcodeNames() {
  std::vector<std::pair<std::string,ExtOpcode>> ret;
  for (size_t i = 0; i < Op_count; i++) {
    ret.push_back(make_pair(extOpcodeToString(ExtOp(i)), i));
  }
  for (ExtOpcode i = ExtOpStart + 1; i < ExtOpEnd; i++) {
    ret.push_back(make_pair(extOpcodeToString(i), i));
  }
  return ret;
}

ExtOpcode stringToExtOpcode(std::string s) {
  static std::vector<std::pair<std::string, ExtOpcode>>
         validOpcodes = getValidOpcodeNames();
  for (size_t i = 0; i < validOpcodes.size(); i++) {
    if (validOpcodes[i].first == s) {
      return validOpcodes[i].second;
    }
  }
  return 0;
}

std::string tcRegionToString(TCRegion tcr) {
  always_assert(tcr < TCRCount);
  return TCRegionString[tcr];
}

/* AddrToBcMapper */

static const size_t kMaxErrStrLen = 1000;
static char errBuff[kMaxErrStrLen];

bool isTraceletGuard(TCA addr, const TransRec* trec) {
  always_assert(!trec->bcMapping.empty());

  if ((addr >= trec->aStart && addr < trec->bcMapping[0].aStart) ||
      (addr >= trec->acoldStart && addr < trec->bcMapping[0].acoldStart)) {
    return true;
  }

  return false;
}

struct ExtOpcodeResult { bool valid; ExtOpcode opcode; };
ExtOpcodeResult getExtOpcode(TCA addr, const TransRec* trec) {

  always_assert(trec);

  if (trec->kind == TransKind::LivePrologue ||
      trec->kind == TransKind::ProfPrologue ||
      trec->kind == TransKind::OptPrologue) {
    return {true, ExtOpFuncPrologue};
  }
  if (isTraceletGuard(addr, trec)) {
    return {true, ExtOpTraceletGuard};
  }

  const std::vector<TransBCMapping>& bcMap = trec->bcMapping;
  always_assert(!bcMap.empty());
  size_t numBCs = bcMap.size() - 1; // account for the sentinel

  for (size_t i = 0; i < numBCs; i++) {
    if ((bcMap[i].aStart       <= addr && bcMap[i+1].aStart       > addr) ||
        (bcMap[i].acoldStart   <= addr && bcMap[i+1].acoldStart   > addr) ||
        (bcMap[i].afrozenStart <= addr && bcMap[i+1].afrozenStart > addr)) {
      auto* unit = g_repo->getUnit(bcMap[i].md5);
      always_assert(unit);
      return {true, (ExtOpcode)unit->getOp(bcMap[i].bcStart)};
    }
  }

  snprintf(errBuff, kMaxErrStrLen, "getExtOpcode: no opcode for %p", addr);
  error(std::string(errBuff));
  return {false};
}

folly::Optional<ExtOpcode>
AddrToBcMapper::operator()(const TCA& addr) {
  TransID tid = transData->getTransContaining(addr);
  if (tid == INVALID_ID) return folly::none;

  const TransRec* trec = transData->getTransRec(tid);
  Unit* unit = g_repo->getUnit(trec->md5);
  if (!unit) return folly::none;

  auto r = getExtOpcode(addr, trec);
  always_assert(r.valid);
  return folly::make_optional(r.opcode);
}

/* AddrToTransFragmentMapper */

TransFragment
AddrToTransFragmentMapper::extractTransFragment(TCA addr, ExtOpcode opcode) {
  TransID tid = tdata->getTransContaining(addr);
  always_assert(tid != INVALID_ID);

  const TransRec* trec = tdata->getTransRec(tid);
  always_assert(!trec->bcMapping.empty());

  TransFragment tfragment;
  tfragment.tid = tid;

  switch (opcode) {
    case ExtOpTraceletGuard:
      tfragment.aStart       = trec->aStart;
      tfragment.aLen         = trec->bcMapping[0].aStart - trec->aStart;
      tfragment.acoldStart   = trec->acoldStart;
      tfragment.acoldLen     = trec->bcMapping[0].acoldStart -
                               trec->acoldStart;
      tfragment.afrozenStart = trec->afrozenStart;
      tfragment.afrozenLen   = trec->bcMapping[0].afrozenStart -
                               trec->afrozenStart;
      break;

    case ExtOpFuncPrologue:
      always_assert(trec->kind == TransKind::LivePrologue ||
                    trec->kind == TransKind::ProfPrologue ||
                    trec->kind == TransKind::OptPrologue);

      tfragment.aStart       = trec->aStart;
      tfragment.aLen         = trec->aLen;
      tfragment.acoldStart   = trec->acoldStart;
      tfragment.acoldLen     = trec->acoldLen;
      tfragment.afrozenStart = trec->afrozenStart;
      tfragment.afrozenLen   = trec->afrozenLen;
      break;

    default:
      bool found  = false;
      size_t nele = trec->bcMapping.size() - 1;

      for (size_t i = 0; i < nele; i++) {
        if ((trec->bcMapping[i].aStart <= addr &&
             addr < trec->bcMapping[i+1].aStart) ||
            (trec->bcMapping[i].acoldStart <= addr &&
             addr < trec->bcMapping[i+1].acoldStart)) {

          found = true;

          tfragment.aStart       = trec->bcMapping[i].aStart;
          tfragment.aLen         = trec->bcMapping[i+1].aStart -
                                   trec->bcMapping[i].aStart;
          tfragment.acoldStart   = trec->bcMapping[i].acoldStart;
          tfragment.acoldLen     = trec->bcMapping[i+1].acoldStart -
                                   trec->bcMapping[i].acoldStart;
          tfragment.afrozenStart = trec->bcMapping[i].afrozenStart;
          tfragment.afrozenLen   = trec->bcMapping[i+1].afrozenStart -
                                   trec->bcMapping[i].afrozenStart;

          break;
        }
      }

      always_assert(found);
  }

  return tfragment;
}

folly::Optional<TransFragment>
AddrToTransFragmentMapper::operator()(const TCA& addr) {
  AddrToBcMapper bcMapper(tdata);
  folly::Optional<ExtOpcode> opcode = bcMapper(addr);
  if (!opcode || (*opcode) != filterBy) return folly::none;
  return extractTransFragment(addr, *opcode);
}


} }
