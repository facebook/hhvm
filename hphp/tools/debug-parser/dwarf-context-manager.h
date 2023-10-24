#pragma once

#include <memory>

#include <folly/Format.h>

#include "llvm/DebugInfo/DWARF/DWARFContext.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Support/MemoryBuffer.h"

namespace debug_parser {

/**
* This class manages the lifetime of an llvm DWARFContext.
*/
class DWARFContextManager {
public:
  explicit DWARFContextManager(const std::string& filename) {
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
    dwarfContext_ = llvm::DWARFContext::create(*objFile_);
    dwoContext_ = dwarfContext_->getDWOContext(filename+".dwp");
  }

  llvm::DWARFContext *getDWARFContext() const { return dwarfContext_.get(); }

  /**
  * Iterate over all units, calling `f()` on each. Iteration is stopped
  * early if any of the calls return false.
  */
  template <typename F> void forEachNormalUnit(F &&f) const;
  template <typename F> void forEachDwoUnit(F &&f) const;
  template <typename F> void forEachUnit(F &&f) const;

private:
  std::unique_ptr<llvm::MemoryBuffer> buffer_;
  std::unique_ptr<llvm::object::ObjectFile> objFile_;
  std::unique_ptr<llvm::DWARFContext> dwarfContext_;
  std::shared_ptr<llvm::DWARFContext> dwoContext_;

};

template <typename F>
void DWARFContextManager::forEachNormalUnit(F &&f) const {
  for (auto& unit : dwarfContext_->normal_units()) {
    if (!f(unit)) return;
  }
}

template <typename F>
void DWARFContextManager::forEachDwoUnit(F &&f) const {
  for (auto& unit : dwoContext_->dwo_units()) {
    if (!f(unit)) return;
  }
}

template <typename F>
void DWARFContextManager::forEachUnit(F &&f) const {
  for (auto& unit : dwarfContext_->normal_units()) {
    if (!f(unit)) return;
  }

  for (auto& unit : dwoContext_->dwo_units()) {
    if (!f(unit)) return;
  }
}

}
