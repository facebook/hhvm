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

#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/util/sha1.h"

namespace HPHP {
struct Func;
struct StringData;

namespace jit {
struct RegionContext;

enum class SBProfTypeSpec : uint8_t {
  None,
  Sub,
  Exact,
};

struct SBProfType {
  Type::bits_t m_bits;
  SBProfTypeSpec m_spec;
  const StringData* m_clsName;
};

struct SBProfTypedLocation {
  SBProfTypedLocation(Location l, SBProfType t)
    : location(l)
    , type(t)
  {}
  Location location;
  SBProfType type;
};

enum class ProfDataSBKind : uint8_t {
  Region,
  Prologue
};

struct ProfDataSBSer {
  ProfDataSBSer(const Func* func, ProfDataSBKind);
  virtual ~ProfDataSBSer() = default;
  ProfDataSBKind m_kind;
  const StringData* m_unitPath;
  // This is the unit that contains the bytecode of the function. If this unit
  // is modified, do not JIT this ProfDataSB.
  // This is serialized separately only if it is different from m_unitPath.
  const StringData* m_bcUnitPath {nullptr};
  SHA1 m_bcUnitSha1 {0};
  const StringData* m_funcName;
  const StringData* m_clsName {nullptr};
  const StringData* m_context {nullptr};
};

struct ProfDataSBRegionSer : ProfDataSBSer {
  explicit ProfDataSBRegionSer(RegionContext& ctx);
  uint32_t m_offsetAndMode;
  std::vector<SBProfTypedLocation> m_liveProfTypes;
  SBInvOffset m_spOffset;
};

struct ProfDataSBPrologueSer : ProfDataSBSer {
  ProfDataSBPrologueSer(const Func* func, int nPassed);
  int m_nPassed;
};

struct ProfDataSBDeser {
  ProfDataSBDeser(Unit* unit, const StringData* funcName,
                  const StringData* clsName, const StringData* context,
                  ProfDataSBKind);
  virtual ~ProfDataSBDeser() = default;
  ProfDataSBKind m_kind;
  Unit* m_unit;
  const StringData* m_funcName;
  const StringData* m_clsName {nullptr};
  const StringData* m_context {nullptr};
};

struct ProfDataSBRegionDeser : ProfDataSBDeser {
  ProfDataSBRegionDeser(Unit* unit,
                        const StringData* funcName, const StringData* clsName,
                        const StringData* context, uint32_t offsetAndMode,
                        const std::vector<SBProfTypedLocation>& liveProfTypes,
                        SBInvOffset sbOffset);
  uint32_t m_offsetAndMode;
  std::vector<SBProfTypedLocation> m_liveProfTypes;
  SBInvOffset m_spOffset;
};

struct ProfDataSBPrologueDeser : ProfDataSBDeser {
  ProfDataSBPrologueDeser(Unit* unit, const StringData* funcName,
                          const StringData* clsName, const StringData* context,
                          int nPassed);
  int m_nPassed;
};

void addToSBProfile(RegionContext& ctx);
void addToSBProfile(const Func* func, int nPassed);

std::vector<ProfDataSBSer*> getSBSerProfDataCopy();
std::vector<std::unique_ptr<ProfDataSBDeser>>& getSBDeserProfData();
}} // namespace HPHP::jit
