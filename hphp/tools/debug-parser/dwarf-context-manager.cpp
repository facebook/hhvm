#include "hphp/tools/debug-parser/dwarf-context-manager.h"

#include <algorithm>
#include <filesystem>

#include "llvm/DebugInfo/DWARF/DWARFDebugInfoEntry.h"
#include "llvm/DebugInfo/DWARF/DWARFUnit.h"
#include "llvm/DebugInfo/DWARF/DWARFTypeUnit.h"

#include "hphp/util/trace.h"

namespace debug_parser {

namespace {
TRACE_SET_MOD(trans);
}

DWARFContextManager::DWARFContextManager(std::string filename) {
  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> dwarfBufferErrorOr =
      llvm::MemoryBuffer::getFileOrSTDIN(filename);

  if (dwarfBufferErrorOr.getError()) {
    auto const msg = dwarfBufferErrorOr.getError().message();
    llvm::errs() << fmt::format("Error reading file ({}): {}\n", filename,
                                msg);
    return;
  }

  buffer_ = std::move(dwarfBufferErrorOr.get());
  llvm::Expected<std::unique_ptr<llvm::object::ObjectFile>> objFile =
      llvm::object::ObjectFile::createObjectFile(buffer_->getMemBufferRef());

  if (!objFile) {
    llvm::errs() << fmt::format("Failed to create object file:{}\n",
                                toString(objFile.takeError()));
    return;
  }


  objFile_ = std::move(objFile.get());

  // LLVM has a threadsafe dwarf context but we only modify the context on the
  // main thread and then access individual units in the worker threads.
  dwarfContext_ = llvm::DWARFContext::create(*objFile_);

  // TODO: see if we can use the main dwarf context instead of handling .dwp in
  // a one-off manner
  if (std::filesystem::exists(filename + ".dwp")) {
    dwoContext_ = dwarfContext_->getDWOContext(filename + ".dwp");
  }

  // Load units into supplementary datastructures
  loadUnits();
}

void DWARFContextManager::loadUnits() {
  // Only create the type signature index if the TU index doesn't already exist
  const auto createIndex = !(dwoContext_ && dwoContext_->getTUIndex());

  const auto processUnit = [&](const std::unique_ptr<llvm::DWARFUnit>& dwarfUnit, bool isInfo) {
    // Pre-load type units to speed up lookups
    if (dwarfUnit->isTypeUnit() || dwoContext_ == nullptr) {
      dwarfUnit->getNonSkeletonUnitDIE(false);
    }

    // Store type unit signatures in a lookup table when TU index doesn't exist
    if (dwarfUnit->isTypeUnit() && createIndex) {
      const uint64_t typeSignature = cast_or_null<llvm::DWARFTypeUnit>(
        dwarfUnit.get())->getTypeHash();
      const uint64_t typeOffset = cast_or_null<llvm::DWARFTypeUnit>(
        dwarfUnit.get())->getTypeOffset();
      sig8Map_.emplace(
        typeSignature,
        getGlobalOffset(dwarfUnit->getOffset() + typeOffset, isInfo)
      );
    }

    (isInfo ? infoUnits_ : typeUnits_).push_back(dwarfUnit.get());

    return true;
  };

  forEachSectionUnit(processUnit, true);
  forEachSectionUnit(processUnit, false);
}

llvm::DWARFUnit* DWARFContextManager::findUnitForGlobalOffset(GlobalOff globalOff) const {
  auto& units = globalOff.isInfo() ? infoUnits_ : typeUnits_;

  auto it = std::upper_bound(units.begin(), units.end(), globalOff.offset(),
    [](uint64_t lhs, const llvm::DWARFUnit* rhs) {
      return lhs < rhs->getNextUnitOffset();
    }
  );

  // We always expect to find the unit for a given offset
  always_assert(it != units.end() && (*it)->getOffset() <= globalOff.offset());
  return *it;
}

DieContext DWARFContextManager::getDieContextAtGlobalOffset(GlobalOff globalOff) const {
  auto dwarfUnit = findUnitForGlobalOffset(globalOff);

  return {
    .die = dwarfUnit->getDIEForOffset(globalOff.offset()),
    .isInfo = globalOff.isInfo()
  };
}

// Specifically do not resolve DW_AT_specification or DW_AT_abstract_origin to
// maintain backwards compatibility with old dwarf parser.
std::string DWARFContextManager::getDIEName(llvm::DWARFDie die) const {
  for (const auto& attr : die.attributes()) {
    if (attr.Attr == llvm::dwarf::DW_AT_name) {
      if (auto val = llvm::dwarf::toString(attr.Value)) {
        return *val;
      }
    }
  }

  return "";
}

GlobalOff DWARFContextManager::getGlobalOffset(uint64_t offset, bool isInfo) const {
  // Treat all offsets as info units, this can be cleaned up as we only need
  // offsets in one file at a time (main binary _or_ dwp)
  const auto hasDwp = dwoContext_ != nullptr;
  return GlobalOff{offset, isInfo, hasDwp};
}

GlobalOff DWARFContextManager::getTypeUnitOffset(uint64_t sig8) const {
  // Use the TU index to retrieve type information, if it exists
  if (dwoContext_ && dwoContext_->getTUIndex()) {
    llvm::DWARFTypeUnit* tu = dwoContext_->getTypeUnitForHash(5, sig8, true);
    if (tu) {
      return getGlobalOffset(tu->getOffset() + tu->getTypeOffset(), true);
    }
  }

  // Otherwise, it will have been populated when units were loaded
  always_assert(sig8Map_.contains(sig8));
  return sig8Map_.at(sig8);
}

}
