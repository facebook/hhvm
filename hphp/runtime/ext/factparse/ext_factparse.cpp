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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/file-stream-wrapper.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/program-functions.h"
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

#include <folly/Conv.h>
#include <folly/MPMCQueue.h>

#include <sys/stat.h>

TRACE_SET_MOD(factparse)

namespace HPHP {
namespace {

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
    hphp_session_init(Treadmill::SessionKind::FactsWorker);
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
    if (useThreads) {
      facts_parse_threaded<HackCFactsExtractor>(root, pathList,
        outResArr, allowHipHopSyntax);
    } else {
      facts_parse_sequential<HackCFactsExtractor>(root, pathList,
        outResArr, allowHipHopSyntax);
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
