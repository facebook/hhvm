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
  auto createIndex = !(dwoContext_ && dwoContext_->getTUIndex());

  auto unitIter = dwoContext_ ? dwoContext_->dwo_units() : dwarfContext_->normal_units();
  for (auto& dwarfUnit : unitIter) {
    // Pre-load type units to speed up lookups
    if (dwarfUnit->isTypeUnit() || dwoContext_ == nullptr) {
      dwarfUnit->getNonSkeletonUnitDIE(false);
    }

    // Store type unit signatures in a lookup table when TU index doesn't exist
    if (dwarfUnit->isTypeUnit() && createIndex) {
      const uint64_t typeSignature = cast_or_null<llvm::DWARFTypeUnit>(dwarfUnit.get())->getTypeHash();
      const uint64_t typeOffset = cast_or_null<llvm::DWARFTypeUnit>(dwarfUnit.get())->getTypeOffset();
      sig8Map_.emplace(
        typeSignature,
        getGlobalOffset(dwarfUnit->getOffset() + typeOffset)
      );
    }

  }
}

llvm::DWARFUnit* DWARFContextManager::findUnitForOffset(uint64_t offset) const {
  auto const& unitVector = dwoContext_ ?
    dwoContext_->getDWOUnitsVector() :
    dwarfContext_->getNormalUnitsVector();

  return unitVector.getUnitForOffset(offset);
}

llvm::DWARFDie DWARFContextManager::getDieAtOffset(uint64_t dieOffset) const {
  // Load the die from the corresponding dwarf unit. This could be made faster
  // by using extractFast(), but then additional state would need to be
  // maintained for dies, walking children, etc.
  auto dwarfUnit = findUnitForOffset(dieOffset);
  const auto die = dwarfUnit->getDIEForOffset(dieOffset);
  return die;
}

// Specifically do not resolve DW_AT_specification or DW_AT_abstract_origin to
// maintain backwards compatibility with old dwarf parser.
std::string DWARFContextManager::getDIEName(llvm::DWARFDie die) const {
  for (const auto& attr : die.attributes()) {
    if (attr.Attr == llvm::dwarf::DW_AT_name) {
      return attr.Value.getAsCString().get();
    }
  }

  return "";
}

GlobalOff DWARFContextManager::getGlobalOffset(uint64_t offset) const {
  // Treat all offsets as info units, this can be cleaned up as we only need
  // offsets in one file at a time (main binary _or_ dwp)
  const auto hasDwp = dwoContext_ != nullptr;
  return GlobalOff{offset, true, hasDwp};
}

GlobalOff DWARFContextManager::getTypeUnitOffset(uint64_t sig8) const {
  // Use the TU index to retrieve type information, if it exists
  if (dwoContext_ && dwoContext_->getTUIndex()) {
    llvm::DWARFTypeUnit* tu = dwoContext_->getTypeUnitForHash(5, sig8, true);
    if (tu) {
      return getGlobalOffset(tu->getOffset() + tu->getTypeOffset());
    }
  }

  // Otherwise, it will have been populated when units were loaded
  always_assert(sig8Map_.contains(sig8));
  return sig8Map_.at(sig8);
}

}
