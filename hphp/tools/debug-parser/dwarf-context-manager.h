#pragma once

#include <memory>

#include "folly/Format.h"
#include "folly/container/F14Map.h"
#include "llvm/DebugInfo/DWARF/DWARFContext.h"
#include "llvm/DebugInfo/DWARF/DWARFDebugInfoEntry.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Support/MemoryBuffer.h"

#include "hphp/tools/debug-parser/dwarf-global-offset.h"

namespace debug_parser {

/**
* This class manages the lifetime of an llvm DWARFContext.
*/
class DWARFContextManager {
public:
  explicit DWARFContextManager(std::string filename);

  llvm::DWARFContext *getDWARFContext() const { return dwarfContext_.get(); }
  llvm::DWARFContext *getDWOContext() const { return dwoContext_.get(); }

  /**
  * Iterate over all units, calling `f()` on each. Iteration is stopped
  * early if any of the calls return false.
  */
  template <typename F> void forEachNormalUnit(F &&f) const;
  template <typename F> void forEachDwoUnit(F &&f) const;
  template <typename F> void forEachUnit(F &&f) const;

  // Provided for backwards compatibility, llvm's `getShortName` will resolve
  // DW_AT_specification or DW_AT_abstract_origin which is not what we want.
  std::string getDIEName(llvm::DWARFDie die) const;
  llvm::DWARFDie getDieAtOffset(uint64_t dieOffset) const;
  GlobalOff getGlobalOffset(uint64_t offset) const;

  GlobalOff getTypeUnitOffset(uint64_t sig8) const;

private:
  std::unique_ptr<llvm::MemoryBuffer> buffer_;
  std::unique_ptr<llvm::object::ObjectFile> objFile_;
  std::unique_ptr<llvm::DWARFContext> dwarfContext_;

  // TODO: placeholders
  std::unique_ptr<llvm::MemoryBuffer> bufferDWP_;
  std::unique_ptr<llvm::object::ObjectFile> objFileDWP_;
  std::shared_ptr<llvm::DWARFContext> dwoContext_;

  // Map from a type signature to the offset of the corresponding type unit.
  // This is used for non-split dwarf when the tu index is not present. For
  // split dwarf we can use `.debug_tu_index`.
  folly::F14FastMap<uint64_t, GlobalOff> sig8Map_;

  // Supplementary function to load units into the unit vector and populate sig8
  // mapping for non-split dwarf usecases.
  void loadUnits();

  // Lookup the the unit that contains the given offset
  llvm::DWARFUnit* findUnitForOffset(uint64_t offset) const;
};

template <typename F>
void DWARFContextManager::forEachNormalUnit(F &&f) const {
  for (const auto& unit : dwarfContext_->normal_units()) {
    if (!f(unit)) return;
  }
}

template <typename F>
void DWARFContextManager::forEachDwoUnit(F &&f) const {
  if (!dwoContext_) return;
  for (const auto& unit : dwoContext_->dwo_units()) {
    if (!f(unit)) return;
  }
}

template <typename F>
void DWARFContextManager::forEachUnit(F &&f) const {
  auto unitIter = dwoContext_ ? dwoContext_->dwo_units() : dwarfContext_->normal_units();
  for (const auto& unit : unitIter) {
    if (!f(unit)) return;
  }
}

}
