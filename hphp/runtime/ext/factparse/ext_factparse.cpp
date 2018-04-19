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
#include "hphp/runtime/ext/json/ext_json.h"
#include "hphp/runtime/server/writer.h"
#include "hphp/runtime/vm/extern-compiler.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/file.h"
#include "hphp/util/process.h"
#include "hphp/util/trace.h"
#include "hphp/util/job-queue.h"

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

template<class T>
void parse_file(
  const std::string& root,
  const char* path,
  bool allowHipHopSyntax,
  typename T::result_type& res,
  const typename T::state_type& state
) {
  T::mark_failed(res);
  std::string cleanPath;
  if (FileUtil::isAbsolutePath(path)) {
    cleanPath = path;
  } else if (root.empty()) {
    cleanPath = path;
  } else {
    cleanPath =
      folly::sformat("{}{}{}", root, FileUtil::getDirSeparator(), path);
  }

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
    T::parse_file_impl(cleanPath,
      allowHipHopSyntax, str.data(), str.size(), res, state);
  } else {
    // It would be nice to have an atomic stat + open operation here but this
    // doesn't seem to be possible with STL in a portable way.
    if (stat(cleanPath.c_str(), &st)) {
      return;
    }
    if (S_ISDIR(st.st_mode)) {
      return;
    }
    T::parse_file_impl(cleanPath, allowHipHopSyntax, "", 0, res, state);
  }
}

template<class T>
void facts_parse_sequential(
  const std::string& root,
  const Array& pathList,
  ArrayInit& outResArr,
  bool allowHipHopSyntax
) {
  const auto state = T::init_state();
  for (auto i = 0; i < pathList->size(); ++i) {
    typename T::result_type workerResult;
    auto path = tvCastToString(pathList->atPos(i));
    try {
      parse_file<T>(root, path.c_str(), allowHipHopSyntax, workerResult, state);
    } catch (...) {
      T::mark_failed(workerResult);
    }
    T::merge_result(workerResult, outResArr, path);
  }
}
template<class T>
struct JobContext {
  JobContext(
    const std::string& root,
    bool allowHipHopSyntax,
    std::vector<std::string>& paths,
    std::vector<typename T::result_type>& worker_results,
    folly::MPMCQueue<size_t>& result_q
  ) : m_root(root), m_allowHipHopSyntax(allowHipHopSyntax), m_paths(paths),
      m_worker_results(worker_results), m_result_q(result_q) {
  }
  const std::string& m_root;
  bool m_allowHipHopSyntax;
  std::vector<std::string>& m_paths;
  std::vector<typename T::result_type>& m_worker_results;
  folly::MPMCQueue<size_t>& m_result_q;
};

template<class T>
using BaseWorker = JobQueueWorker<size_t, const JobContext<T>*, false, true>;

template<class T>
struct ParseFactsWorker: public BaseWorker<T> {
  void doJob(typename BaseWorker<T>::JobType job) override {
    parse(m_state, *(this->m_context), job);
  }
  void onThreadEnter() override {
    hphp_session_init();
    m_state = T::init_state();
  }
  void onThreadExit() override {
    hphp_context_exit();
    hphp_session_exit();
  }
  static void parse(const typename T::state_type& state,
                    const JobContext<T>& ctx, size_t i) {
    try {
      parse_file<T>(
        ctx.m_root,
        ctx.m_paths[i].c_str(),
        ctx.m_allowHipHopSyntax,
        ctx.m_worker_results[i],
        state);
    } catch (...) {
      T::mark_failed(ctx.m_worker_results[i]);
    }
    ctx.m_result_q.write(i);
  }
private:
  typename T::state_type m_state;
};

template<class T>
void facts_parse_threaded(
  const std::string& root,
  const Array& pathList,
  ArrayInit& outResArr,
  bool allowHipHopSyntax
) {
  auto numPaths = pathList->size();
  std::vector<std::string> pathListCopy;
  for(auto i = 0; i < numPaths; i++) {
    pathListCopy.push_back(
      tvCastToString(pathList->atPos(i)).toCppString()
    );
  }
  std::vector<typename T::result_type> workerResults;
  workerResults.resize(numPaths);
  folly::MPMCQueue<size_t> result_q{numPaths};

  JobContext<T> jobContext { root, allowHipHopSyntax,
    pathListCopy, workerResults, result_q };
  JobQueueDispatcher<ParseFactsWorker<T>> dispatcher {
    T::get_workers_count(), T::get_workers_count(), 0, false, &jobContext
  };
  dispatcher.start();

  for (auto i = 0; i < numPaths; ++i) {
    dispatcher.enqueue(i);
  }

  for (auto numProcessed = 0; numProcessed < numPaths; ++numProcessed) {
    size_t i;
    result_q.blockingRead(i);
    T::merge_result(
      workerResults[i],
      outResArr,
      tvCastToString(pathList->atPos(i)));
  }
  dispatcher.waitEmpty();
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

struct HHVMFactsExtractor {
  using result_type = Facts::ParseResult;
  using state_type = void*;

  static int get_workers_count() {
    return Process::GetCPUCount();
  }

  static state_type init_state() {
    return nullptr;
  }

  static void parse_stream(
    std::istream& stream,
    const std::string& path,
    bool allowHipHopSyntax,
    result_type& res) {
    if (!stream.good()) {
      return;
    }
    Scanner scanner(
      stream,
      allowHipHopSyntax ? Scanner::Type::AllowHipHopSyntax : 0,
      path.c_str(),
      true);
    Facts::Parser parser(scanner, path.c_str(), res);
    try {
      parser.parse();
    } catch (ParseTimeFatalException& e) {
      FTRACE(1, "Parse fatal @{}: {}\n", e.m_line, e.getMessage());
    }
  }

  static void parse_file_impl(
    const std::string& path,
    bool allowHipHopSyntax,
    const char* code,
    int len,
    result_type& res,
    const state_type&
  ) {
    if (len == 0) {
      std::ifstream stream(path);
      parse_stream(stream, path, allowHipHopSyntax, res);
    }
    else {
      std::string content(code, len);
      std::istringstream stream(content);
      parse_stream(stream, path, allowHipHopSyntax, res);
    }
  };

  static void mark_failed(result_type& workerResult) {
    workerResult.error = true;
  }

  static void merge_result(
    result_type& workerResult,
    ArrayInit& outResArr,
    const HPHP::String& path
  )  {
    buildOneResult(workerResult, outResArr, path);
  };
};

struct HackCFactsExtractor {
  using result_type = folly::Optional<FactsJSONString>;
  using state_type = std::unique_ptr<FactsParser>;

  static int get_workers_count() {
    return RuntimeOption::EvalHackCompilerWorkers;
  }

  static state_type init_state() {
    return acquire_facts_parser();
  }

  static void mark_failed(result_type& workerResult) {
    workerResult = folly::none;
  }

  static void parse_file_impl(
    const std::string& path,
    bool allowHipHopSyntax,
    const char* code,
    int len,
    result_type& res,
    const state_type& state
  ) {
    auto result = extract_facts(*state, path, code, len);
    match<void>(
      result,
      [&](FactsJSONString& r) {
        res = std::move(r);
      },
      [&](std::string& err) {
        FTRACE(1, "Error extracting facts {}: {}\n", path, err);
      }
    );
  };

  static void merge_result(
    result_type& workerResult,
    ArrayInit& outResArr,
    const HPHP::String& path
  ) {
    try {
      if (workerResult.hasValue() && !workerResult.value().value.empty()) {
        outResArr.set(
          path,
          f_json_decode(String(workerResult.value().value), true, 512,
            k_JSON_FB_LOOSE));
      }
      else {
        outResArr.set(path, uninit_null());
      }
    }
    catch (std::runtime_error& e) {
      FTRACE(1, "Error during json conversion {}: {}\n", path, e.what());
    }
  };
};

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
    auto useHackc = hackc_mode() != HackcMode::kNever;
    if (useThreads) {
      if (useHackc) {
        facts_parse_threaded<HackCFactsExtractor>(root, pathList,
          outResArr, allowHipHopSyntax);
      }
      else {
        facts_parse_threaded<HHVMFactsExtractor>(root, pathList,
          outResArr, allowHipHopSyntax);
      }
    } else {
      if (useHackc) {
        facts_parse_sequential<HackCFactsExtractor>(root, pathList,
          outResArr, allowHipHopSyntax);
      }
      else {
        facts_parse_sequential<HHVMFactsExtractor>(root, pathList,
          outResArr, allowHipHopSyntax);
      }
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
