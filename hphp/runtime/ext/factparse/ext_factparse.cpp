/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/factparse/parser.h"

#include <sys/stat.h>

#include <fstream>

#include <folly/Conv.h>
#include <folly/MPMCQueue.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/config.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/file-stream-wrapper.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/server/writer.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/file.h"
#include "hphp/util/process.h"
#include "hphp/util/trace.h"

TRACE_SET_MOD(factparse)

namespace HPHP {
namespace {

const StaticString
  s_md5sum0("md5sum0"),
  s_md5sum1("md5sum1"),
  s_types("types"),
  s_constants("constants"),
  s_functions("functions"),
  s_typeAliases("typeAliases"),
  s_name("name"),
  s_kindOf("kindOf"),
  s_baseTypes("baseTypes"),
  s_class("class"),
  s_interface("interface"),
  s_enum("enum"),
  s_trait("trait"),
  s_unknown("unknown"),
  s_mixed("mixed"),
  s_flags("flags"),
  s_requireImplements("requireImplements"),
  s_requireExtends("requireExtends");

const StaticString* g_factKindOfMap[] = {
  #define FACT_KIND_MAP(x, y) &s_ ## y
  FACT_KIND_GEN(FACT_KIND_MAP)
  #undef FACT_KIND_MAP
};

void parseFile(
  const std::string& root,
  const char* path,
  Facts::ParseResult& res,
  bool allowHipHopSyntax)
{
  res.error = true;
  std::string cleanPath;
  if (FileUtil::isAbsolutePath(path)) {
    cleanPath = path;
  } else if (root.empty()) {
    cleanPath = path;
  } else {
    cleanPath =
      folly::sformat("{}{}{}", root, FileUtil::getDirSeparator(), path);
  }
  auto parse = [&] (std::istream& stream) {
    if (!stream.good()) {
      return;
    }
    Scanner scanner(
      stream,
      allowHipHopSyntax ? Scanner::Type::AllowHipHopSyntax : 0,
      path,
      true);
    Facts::Parser parser(scanner, path, res);
    try {
      parser.parse();
    } catch (ParseTimeFatalException& e) {
      FTRACE(1, "Parse fatal @{}: {}\n", e.m_line, e.getMessage());
    }
  };

  struct stat st;
  auto w = Stream::getWrapperFromURI(StrNR(cleanPath));
  if (w && !dynamic_cast<FileStreamWrapper*>(w)) {
    if (w->stat(cleanPath.c_str(), &st)) {
      return;
    }
    if (S_ISDIR(st.st_mode)) {
      return;
    }
    const auto f = w->open(StrNR(cleanPath), "r", 0, nullptr);
    if (!f) return;
    auto str = f->read();
    std::string content(str.data(), str.size());
    std::istringstream stream(content);
    parse(stream);
  } else {
    // It would be nice to have an atomic stat + open operation here but this
    // doesn't seem to be possible with STL in a portable way.
    if (stat(cleanPath.c_str(), &st)) {
      return;
    }
    if (S_ISDIR(st.st_mode)) {
      return;
    }
    std::ifstream stream(cleanPath);
    parse(stream);
  }
}

void buildOneResult(
  Facts::ParseResult& inRes,
  ArrayInit& outResArr,
  const HPHP::String& path)
{
  if (inRes.error) {
    outResArr.set(path, uninit_null());
    return;
  }

  // Re-pack types
  PackedArrayInit resTypes(inRes.typesMap.size());
  for (auto& name_type : inRes.typesMap) {
    auto type = name_type.second;
    int typeDetailsFields = 4;
    if (type.kindOf == Facts::FactKindOf::Trait) {
      typeDetailsFields += 2;
    } else if (type.kindOf == Facts::FactKindOf::Interface) {
      typeDetailsFields++;
    }
    ArrayInit typeDetails(typeDetailsFields, ArrayInit::Map{});

    typeDetails.add(s_name, String(name_type.first));
    typeDetails.add(s_kindOf, String(*g_factKindOfMap[(int)type.kindOf]));
    typeDetails.add(s_flags, Variant(type.flags));

    PackedArrayInit resBaseTypes(type.baseTypes.size());
    for (auto& baseType : type.baseTypes) {
      resBaseTypes.append(String(baseType));
    }
    typeDetails.add(s_baseTypes, resBaseTypes.toArray());

    switch (type.kindOf) {
      case Facts::FactKindOf::Trait: {
        PackedArrayInit resRequireImplements(type.requireImplements.size());
        for (auto& t : type.requireImplements) {
          resRequireImplements.append(String(t));
        }
        typeDetails.add(s_requireImplements, resRequireImplements.toArray());
      }
      // FALLTHROUGH

      case Facts::FactKindOf::Interface: {
        PackedArrayInit resRequireExtends(type.requireExtends.size());
        for (auto& t : type.requireExtends) {
          resRequireExtends.append(String(t));
        }
        typeDetails.add(s_requireExtends, resRequireExtends.toArray());
      }
      default: ;
    }
    resTypes.append(typeDetails.toArray());
  }

  // Re-pack functions
  PackedArrayInit resFunctions(inRes.functions.size());
  for (auto& function : inRes.functions) {
    resFunctions.append(String(function));
  }

  // Re-pack constants
  PackedArrayInit resConstants(inRes.constants.size());
  for (auto& constant : inRes.constants) {
    resConstants.append(String(constant));
  }

  // Re-pack type aliases
  PackedArrayInit resTypeAliases(inRes.typeAliases.size());
  for (auto& typeAlias : inRes.typeAliases) {
    resTypeAliases.append(String(typeAlias));
  }

  auto resArr = make_map_array(
    s_md5sum0, reinterpret_cast<int64_t&>(inRes.md5sum[0]),
    s_md5sum1, reinterpret_cast<int64_t&>(inRes.md5sum[1]),
    s_types, resTypes.toArray(),
    s_functions, resFunctions.toArray(),
    s_constants, resConstants.toArray(),
    s_typeAliases, resTypeAliases.toArray());

  outResArr.set(path, resArr);
}

void facts_parse_impl(
  const std::string& root,
  const Array& pathList,
  ArrayInit& outResArr,
  bool allowHipHopSyntax
) {
  for (auto i = 0; i < pathList->size(); ++i) {
    Facts::ParseResult workerResult;
    auto path = tvCastToString(pathList->atPos(i));
    try {
      parseFile(root, path.c_str(), workerResult, allowHipHopSyntax);
    } catch (...) {
      workerResult.error = true;
    }
    buildOneResult(workerResult, outResArr, path);
  }
}

void facts_parse_impl_threaded(
  const std::string& root,
  const Array& pathList,
  ArrayInit& outResArr,
  bool allowHHSyntax
) {
  auto numPaths = pathList->size();
  std::vector<Facts::ParseResult> workerResults;
  workerResults.resize(numPaths);
  std::vector<std::thread> workers;
  auto numWorkers = Process::GetCPUCount();
  // Compute a batch size that causes each thread to process approximately 16
  // batches.  Even if the batches are somewhat imbalanced in what they contain,
  // the straggler workers are very unlikey to take more than 10% longer than
  // the first worker to finish.
  size_t batchSize{std::max(numPaths / numWorkers / 16, size_t(1))};
  std::atomic<size_t> index{0};
  folly::MPMCQueue<size_t> result_q{numPaths};

  std::vector<std::string> pathListCopy;
  for(auto i = 0; i < numPaths; i++) {
    pathListCopy.push_back(
      tvCastToString(pathList->atPos(i)).toCppString()
    );
  }

  for (auto worker = 0; worker < numWorkers; ++worker) {
    workers.push_back(std::thread([&] {
      hphp_thread_init();
      SCOPE_EXIT {
        hphp_thread_exit();
      };
      hphp_session_init();
      SCOPE_EXIT {
        hphp_context_exit();
        hphp_session_exit();
      };

      while (true) {
        auto begin = index.fetch_add(batchSize);
        auto end = std::min(begin + batchSize, numPaths);
        if (begin >= end) {
          break;
        }
        auto unitCount = end - begin;
        for (auto i = 0; i < unitCount; ++i) {
          const auto& path = pathListCopy[begin + i];
          const auto overallIdx = begin + i;
          try {
            parseFile(
              root,
              path.c_str(),
              workerResults[overallIdx],
              allowHHSyntax);
          } catch (...) {
            workerResults[overallIdx].error = true;
          }
          result_q.write(overallIdx);
        }
      }
    }));
  }

  for (auto numProcessed = 0; numProcessed < numPaths; ++numProcessed) {
    size_t i;
    result_q.blockingRead(i);
    buildOneResult(
      workerResults[i],
      outResArr,
      tvCastToString(pathList->atPos(i)));
  }

  for (auto& worker : workers) {
    worker.join();
  }
}

Array HHVM_FUNCTION(
  HH_facts_parse,
  const Variant& _root,
  const Array& pathList,
  bool allowHipHopSyntax,
  bool useThreads
) {
  if (RuntimeOption::RepoAuthoritative) {
    SystemLib::throwInvalidOperationExceptionObject(
      "HH\\facts_parse not allowed in repo-authoritative mode. See #11153611.");
  }

  auto root = _root.isNull() ?
    "" : _root.toString().toCppString();

  ArrayInit outResArr(pathList->size(), ArrayInit::Map{});

  if (!pathList.isNull() && pathList->size()) {
    if (useThreads) {
      facts_parse_impl_threaded(root, pathList, outResArr, allowHipHopSyntax);
    } else {
      facts_parse_impl(root, pathList, outResArr, allowHipHopSyntax);
    }
  }

  return outResArr.toArray();
}

struct FactparseExtension : Extension {
  // See ext_factparse.php for version number bumps overview.
  FactparseExtension() : Extension("factparse", "3") {}
  void moduleInit() override {
    HHVM_FALIAS(HH\\facts_parse, HH_facts_parse);
    loadSystemlib();
  }
};

FactparseExtension s_factparse;

} // namespace
} // namespace HPHP
