/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/file-stream-wrapper.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/req-vector.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/json/ext_json.h"
#include "hphp/runtime/vm/extern-compiler.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/util/file.h"
#include "hphp/util/trace.h"
#include "hphp/util/job-queue.h"

#include <folly/MPMCQueue.h>

#include <sys/stat.h>

TRACE_SET_MOD(factparse)

namespace HPHP {
namespace {

struct HackCFactsExtractor {
  using result_type = Optional<FactsJSONString>;
  using state_type = std::unique_ptr<FactsParser>;

  static int get_workers_count() {
    return RuntimeOption::EvalHackCompilerWorkers;
  }

  static state_type init_state() {
    return acquire_facts_parser();
  }

  static state_type clear_state() {
    return std::unique_ptr<FactsParser>();
  }

  static void mark_failed(result_type& workerResult) {
    workerResult = std::nullopt;
  }

  static void parse_file_impl(
    const std::string& path,
    bool allowHipHopSyntax,
    folly::StringPiece code,
    result_type& res,
    const state_type& state
  ) {
    auto result = extract_facts(*state, path, code.data(), code.size(),
                                RepoOptions::forFile(path.data()));
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
    DArrayInit& outResArr,
    const HPHP::String& path
  ) {
    try {
      if (workerResult.has_value() && !workerResult.value().value.empty()) {
        outResArr.set(
          path,
          Variant::attach(
            HHVM_FN(json_decode)(
              String(workerResult.value().value),
              true,
              512,
              k_JSON_FB_LOOSE | k_JSON_FB_DARRAYS_AND_VARRAYS
            )
          )
        );
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

void parse_file(
  const std::string& root,
  folly::StringPiece path,
  bool allowHipHopSyntax,
  HackCFactsExtractor::result_type& res,
  const HackCFactsExtractor::state_type& state
) {
  HackCFactsExtractor::mark_failed(res);
  std::string cleanPath;
  if (FileUtil::isAbsolutePath(path)) {
    cleanPath = std::string{path};
  } else if (root.empty()) {
    cleanPath = std::string{path};
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
    HackCFactsExtractor::parse_file_impl(cleanPath, allowHipHopSyntax,
                                         str.slice(), res, state);
  } else {
    // It would be nice to have an atomic stat + open operation here but this
    // doesn't seem to be possible with STL in a portable way.
    if (stat(cleanPath.c_str(), &st)) {
      return;
    }
    if (S_ISDIR(st.st_mode)) {
      return;
    }
    HackCFactsExtractor::parse_file_impl(cleanPath, allowHipHopSyntax,
                                         folly::StringPiece{""}, res, state);
  }
}

void facts_parse_sequential(
  const std::string& root,
  const req::vector<StringData*>& pathList,
  DArrayInit& outResArr,
  bool allowHipHopSyntax
) {
  const auto state = HackCFactsExtractor::init_state();
  for (auto i = 0; i < pathList.size(); ++i) {
    HackCFactsExtractor::result_type workerResult;
    auto path = pathList.at(i);
    try {
      parse_file(root, path->slice(), allowHipHopSyntax, workerResult, state);
    } catch (...) {
      HackCFactsExtractor::mark_failed(workerResult);
    }
    HackCFactsExtractor::merge_result(workerResult, outResArr,
                                      StrNR(path));
  }
}
struct JobContext {
  JobContext(
    const std::string& root,
    bool allowHipHopSyntax,
    const req::vector<StringData*>& paths,
    std::vector<HackCFactsExtractor::result_type>& worker_results,
    folly::MPMCQueue<size_t>& result_q
  ) : m_root(root), m_allowHipHopSyntax(allowHipHopSyntax), m_paths(paths),
      m_worker_results(worker_results), m_result_q(result_q) {
  }
  const std::string& m_root;
  bool m_allowHipHopSyntax;
  const req::vector<StringData*>& m_paths;
  std::vector<HackCFactsExtractor::result_type>& m_worker_results;
  folly::MPMCQueue<size_t>& m_result_q;
};

using BaseWorker = JobQueueWorker<size_t, const JobContext*, false, true>;

struct ParseFactsWorker: public BaseWorker {
  void doJob(BaseWorker::JobType job) override {
    parse(m_state, *(this->m_context), job);
  }
  void onThreadEnter() override {
    hphp_session_init(Treadmill::SessionKind::FactsWorker);
    m_state = HackCFactsExtractor::init_state();
  }
  void onThreadExit() override {
    m_state = HackCFactsExtractor::clear_state();
    hphp_context_exit();
    hphp_session_exit();
  }
  static void parse(const HackCFactsExtractor::state_type& state,
                    const JobContext& ctx, size_t i) {
    try {
      parse_file(
        ctx.m_root,
        ctx.m_paths[i]->slice(),
        ctx.m_allowHipHopSyntax,
        ctx.m_worker_results[i],
        state);
    } catch (...) {
      HackCFactsExtractor::mark_failed(ctx.m_worker_results[i]);
    }
    ctx.m_result_q.write(i);
  }
private:
  HackCFactsExtractor::state_type m_state;
};

void facts_parse_threaded(
  const std::string& root,
  const req::vector<StringData*>& pathList,
  DArrayInit& outResArr,
  bool allowHipHopSyntax
) {
  auto numPaths = pathList.size();
  std::vector<HackCFactsExtractor::result_type> workerResults;
  workerResults.resize(numPaths);
  folly::MPMCQueue<size_t> result_q{numPaths};

  JobContext jobContext { root, allowHipHopSyntax,
    pathList, workerResults, result_q };
  JobQueueDispatcher<ParseFactsWorker> dispatcher {
    HackCFactsExtractor::get_workers_count(),
      HackCFactsExtractor::get_workers_count(), 0, false, &jobContext};
  dispatcher.start();

  for (auto i = 0; i < numPaths; ++i) {
    dispatcher.enqueue(i);
  }

  for (auto numProcessed = 0; numProcessed < numPaths; ++numProcessed) {
    size_t i;
    result_q.blockingRead(i);
    HackCFactsExtractor::merge_result(
      workerResults[i],
      outResArr,
      StrNR(pathList.at(i)));
  }
  dispatcher.waitEmpty();
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

  DArrayInit outResArr(pathList->size());

  if (pathList.isNull() || !pathList->size()) {
    return outResArr.toArray();
  }

  auto root = _root.isNull() ?
    "" : _root.toString().toCppString();

  req::vector<StringData*> pathListStringData;
  IterateV(
    pathList.get(),
    [&] (TypedValue tv) {
      if (UNLIKELY(!isStringType(tv.m_type))) {
        SystemLib::throwInvalidOperationExceptionObject(
          "HH\\facts_parse expects a varray<string> but was given an array "
          "with a non-string value."
        );
      }
      pathListStringData.push_back(tv.m_data.pstr);
    }
  );

  if (useThreads) {
    facts_parse_threaded(root, pathListStringData,
      outResArr, allowHipHopSyntax);
  } else {
    facts_parse_sequential(root, pathListStringData,
      outResArr, allowHipHopSyntax);
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
