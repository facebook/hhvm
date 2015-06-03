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

#include "hphp/runtime/vm/jit/trans-rec.h"

#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/mc-generator.h"

#include <unordered_map>

namespace HPHP { namespace jit {

TransRec::TransRec(SrcKey                      _src,
                   TransKind                   _kind,
                   TCA                         _aStart,
                   uint32_t                    _aLen,
                   TCA                         _acoldStart,
                   uint32_t                    _acoldLen,
                   TCA                         _afrozenStart,
                   uint32_t                    _afrozenLen,
                   RegionDescPtr               region,
                   std::vector<TransBCMapping> _bcMapping,
                   Annotations&&               _annotations,
                   bool                        _isLLVM,
                   bool                        _hasLoop)
  : bcMapping(_bcMapping)
  , annotations(std::move(_annotations))
  , funcName(_src.func()->fullName()->data())
  , src(_src)
  , md5(_src.func()->unit()->md5())
  , aStart(_aStart)
  , acoldStart(_acoldStart)
  , afrozenStart(_afrozenStart)
  , aLen(_aLen)
  , acoldLen(_acoldLen)
  , afrozenLen(_afrozenLen)
  , bcStart(_src.offset())
  , id(0)
  , kind(_kind)
  , isLLVM(_isLLVM)
  , hasLoop(_hasLoop)
{
  if (funcName.empty()) funcName = "Pseudo-main";

  if (!region) return;

  assertx(!region->empty());
  for (auto& block : region->blocks()) {
    auto sk = block->start();
    blocks.emplace_back(Block{sk.unit()->md5(), sk.offset(),
                              block->last().advanced().offset()});
  }

  auto& firstBlock = *region->blocks().front();
  auto guardRange = firstBlock.typePreConditions().equal_range(
      firstBlock.start());
  for (; guardRange.first != guardRange.second; ++guardRange.first) {
    guards.emplace_back(show(guardRange.first->second));
  }
}

void
TransRec::optimizeForMemory() {
  // Dump large annotations to disk.
  for (int i = 0 ; i < annotations.size(); ++i) {
    auto& annotation = annotations[i];
    if (annotation.second.find_first_of('\n') != std::string::npos ||
        annotation.second.size() > 72) {
      auto saved = writeAnnotation(annotation, /* compress */ true);
      if (saved.length > 0) {
        // Strip directory name from the file name.
        size_t pos = saved.fileName.find_last_of('/');
        if (pos != std::string::npos) {
          saved.fileName = saved.fileName.substr(pos+1);
        }
        auto newAnnotation =
          folly::sformat(
            "file:{}:{}:{}",
            saved.fileName, saved.offset, saved.length);
        std::swap(annotation.second, newAnnotation);
      } else {
        annotation.second = "<unknown: write failed>";
      }
    }
  }
}

TransRec::SavedAnnotation
TransRec::writeAnnotation(const Annotation& annotation, bool compress) {
  static std::unordered_map<std::string, bool> fileWritten;
  SavedAnnotation saved = {
    folly::sformat("/tmp/tc_annotations.txt{}", compress ? ".gz" : ""),
    0,
    0
  };
  auto const fileName = saved.fileName.c_str();

  auto result = fileWritten.find(saved.fileName);
  if (result == fileWritten.end()) {
    unlink(fileName);
    fileWritten[saved.fileName] = true;
  }

  FILE* file = fopen(fileName, "a");
  if (!file) return saved;
  saved.offset = lseek(fileno(file), 0, SEEK_END);
  if (saved.offset == (off_t)-1) {
    fclose(file);
    return saved;
  }
  auto const& content = annotation.second;
  if (compress) {
    gzFile compressedFile = gzdopen(fileno(file), "a");
    if (!compressedFile) {
      fclose(file);
      return saved;
    }
    auto rv = gzputs(compressedFile, content.c_str());
    if (rv > 0) saved.length = rv;
    gzclose(compressedFile);
  } else {
    if (fputs(content.c_str(), file) >= 0) {
      saved.length = content.length();
    }
    fclose(file);
  }

  return saved;
}

std::string
TransRec::print(uint64_t profCount) const {
  std::string ret;
  std::string funcName = src.func()->fullName()->data();

  // Split up the call to prevent template explosion
  folly::format(
    &ret,
    "Translation {} {{\n"
    "  src.md5 = {}\n"
    "  src.funcId = {}\n"
    "  src.funcName = {}\n"
    "  src.resumed = {}\n"
    "  src.bcStart = {}\n"
    "  src.blocks = {}\n",
    id, md5, src.funcID(),
    funcName.empty() ? "Pseudo-main" : funcName,
    (int32_t)src.resumed(),
    src.offset(),
    blocks.size());

  for (auto const& block : blocks) {
    folly::format(
      &ret,
      "    {} {} {}\n",
      block.md5, block.bcStart, block.bcPast);
  }

  folly::format( &ret, "  src.guards = {}\n", guards.size());

  for (auto const& guard : guards) {
    folly::format( &ret, "    {}\n", guard);
  }

  folly::format(
    &ret,
    "  kind = {} ({})\n"
    "  isLLVM = {:d}\n"
    "  hasLoop = {:d}\n"
    "  aStart = {}\n"
    "  aLen = {:#x}\n"
    "  coldStart = {}\n"
    "  coldLen = {:#x}\n"
    "  frozenStart = {}\n"
    "  frozenLen = {:#x}\n",
    static_cast<uint32_t>(kind), show(kind),
    isLLVM, hasLoop,
    aStart, aLen,
    acoldStart, acoldLen,
    afrozenStart, afrozenLen);

  folly::format(&ret, "  annotations = {}\n", annotations.size());
  for (auto const& annotation : annotations) {
    folly::format(&ret, "     [\"{}\"] = {}\n",
                  annotation.first, annotation.second);
  }

  folly::format(
    &ret,
    "  profCount = {}\n"
    "  bcMapping = {}\n",
    profCount, bcMapping.size());

  for (auto const& info : bcMapping) {
    folly::format(
      &ret,
      "    {} {} {} {} {}\n",
      info.md5, info.bcStart,
      info.aStart, info.acoldStart, info.afrozenStart);
  }

  ret += "}\n\n";
  return ret;
}

Offset TransRec::bcPast() const {
  return blocks.empty() ? 0 : blocks.back().bcPast;
}

} }
