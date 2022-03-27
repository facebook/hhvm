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

#pragma once

#include "hphp/compiler/option.h"

#include "hphp/hhbbc/hhbbc.h"

#include "hphp/runtime/vm/as.h"
#include "hphp/runtime/vm/unit-emitter.h"

#include "hphp/util/mutex.h"

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

struct Package;
struct AnalysisResult;
using AnalysisResultPtr = std::shared_ptr<AnalysisResult>;
using AnalysisResultConstRawPtr = const AnalysisResult*;

struct UnitEmitter;

struct AnalysisResult : std::enable_shared_from_this<AnalysisResult> {
  AnalysisResult();
  ~AnalysisResult();
  void setFinish(std::function<void(AnalysisResultPtr)>&& fn) {
    m_finish = std::move(fn);
  }
  void finish();

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
  HHBBC::php::ProgramPtr& program() { return m_program; }
private:
  std::function<void(AnalysisResultPtr)> m_finish;
  std::vector<std::unique_ptr<UnitEmitter>> m_hhasFiles;
  std::string m_outputPath;
  HHBBC::php::ProgramPtr m_program;

  Mutex m_mutex;
 };

///////////////////////////////////////////////////////////////////////////////
}
