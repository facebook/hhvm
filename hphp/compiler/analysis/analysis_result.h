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

#ifndef incl_HPHP_ANALYSIS_RESULT_H_
#define incl_HPHP_ANALYSIS_RESULT_H_

#include "hphp/compiler/option.h"
#include "hphp/compiler/package.h"

#include "hphp/runtime/vm/as.h"

#include "hphp/util/compact-vector.h"
#include "hphp/util/thread-local.h"

#include <tbb/concurrent_hash_map.h>
#include <atomic>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>
#include <functional>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct AnalysisResult;
using AnalysisResultPtr = std::shared_ptr<AnalysisResult>;
using AnalysisResultConstRawPtr = const AnalysisResult*;

struct UnitEmitter;

struct AnalysisResult : std::enable_shared_from_this<AnalysisResult> {
  /**
   * There are multiple passes over our syntax trees. This lists all of them.
   */
  enum Phase {
    // parse
    ParseAllFiles,

    // analyzeProgram
    AnalyzeAll,
    AnalyzeFinal,

    // pre-optimize
    FirstPreOptimize,

    CodeGen,
  };

  struct Locker {
    explicit Locker(const AnalysisResult *ar) :
        m_ar(const_cast<AnalysisResult*>(ar)),
        m_mutex(m_ar->getMutex()) {
      m_mutex.lock();
    }
    Locker(const Locker &l) : m_ar(l.m_ar), m_mutex(l.m_mutex) {
      const_cast<Locker&>(l).m_ar = 0;
    }
    ~Locker() {
      if (m_ar) m_mutex.unlock();
    }
    AnalysisResult *operator->() const {
      return m_ar;
    }
  private:
    AnalysisResult *m_ar;
    Mutex &m_mutex;
  };

  struct ParseOnDemandCalbacks : AsmCallbacks {
    explicit ParseOnDemandCalbacks(AnalysisResultConstRawPtr ar) : m_ar(ar) {}

    virtual void onInclude(const std::string& include) override {
      m_ar->parseOnDemand(include);
    }
    virtual void onConstantRef(const std::string& constant) override {
      m_ar->parseOnDemandByConstant(constant);
    }
    virtual void onFunctionRef(const std::string& function) override {
      m_ar->parseOnDemandByFunction(function);
    }
    virtual void onClassRef(const std::string& cls) override {
      m_ar->parseOnDemandByClass(cls);
    }

   private:
    AnalysisResultConstRawPtr m_ar;
  };

public:
  AnalysisResult();
  ~AnalysisResult();
  Locker lock() const { return Locker(this); }
  void setPackage(Package *package) { m_package = package;}
  void setParseOnDemand(bool v) { m_parseOnDemand = v;}
  bool isParseOnDemand() const { return m_package && m_parseOnDemand;}
  void setParseOnDemandDirs(const std::vector<std::string> &dirs) {
    assert(m_package && !m_parseOnDemand);
    m_parseOnDemandDirs = dirs;
  }
  void setFinish(std::function<void(AnalysisResultPtr)>&& fn) {
    m_finish = std::move(fn);
  }
  void finish();

  Mutex &getMutex() { return m_mutex; }

  /**
   * Parser creates a FileScope upon parsing a new file.
   */
  void parseOnDemand(const std::string &name) const;
  void parseOnDemandByClass(const std::string &name) const {
    parseOnDemandBy(name, Option::AutoloadClassMap);
  }
  void parseOnDemandByFunction(const std::string &name) const {
    parseOnDemandBy(name, Option::AutoloadFuncMap);
  }
  void parseOnDemandByConstant(const std::string &name) const {
    parseOnDemandBy(name, Option::AutoloadConstMap);
  }
  template <class Map>
  void parseOnDemandBy(const std::string &name,
                       const Map& amap) const;
  ParseOnDemandCalbacks* getParseOnDemandCallBacks() {
    if (isParseOnDemand()) {
      return &m_asmCallbacks;
    }

    return nullptr;
  }

  /**
   * For function declaration parsing.
   */
  static std::string prepareFile(const char *root, const std::string &fileName,
                                 bool chop, bool stripPath = true);

  void setOutputPath(const std::string &path) {
    m_outputPath = path;
  }
  const std::string &getOutputPath() {
    return m_outputPath;
  }

  void addHhasFile(std::unique_ptr<UnitEmitter>&& ue);
  std::vector<std::unique_ptr<UnitEmitter>> getHhasFiles();
private:
  std::function<void(AnalysisResultPtr)> m_finish;
  Package *m_package;
  bool m_parseOnDemand;
  std::vector<std::string> m_parseOnDemandDirs;
  std::vector<std::unique_ptr<UnitEmitter>> m_hhasFiles;

  std::string m_outputPath;

  ParseOnDemandCalbacks m_asmCallbacks;

  Mutex m_mutex;

  /**
   * Checks whether the file is in one of the on-demand parsing directories.
   */
  bool inParseOnDemandDirs(const std::string &filename) const;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_ANALYSIS_RESULT_H_
