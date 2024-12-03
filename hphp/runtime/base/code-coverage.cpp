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
#include "hphp/runtime/base/code-coverage.h"

#include <fstream>
#include <vector>

#include <folly/String.h>

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/ext/extension.h"

#include "hphp/util/logger.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * The function below will be called by the interpreter on each
 * evaluated expression when running hhvm with:
 *   $ hhvm -v Eval.EnableCodeCoverage=1 <phpfile>
 *
 * The line is the 1-indexed line number of the evaluated expression.
 *
 * Indeed on this toy cover.php file:
 *
 *  <?hh
 *
 *  echo "here\n";
 *  for($i = 0; $i < 2; $i++) {
 *    echo "here $i\n";
 *  }
 *
 *  echo 1 +
 *       2 +
 *       (3 + 4);
 *
 *  eval("echo 'bar\n';");
 *
 * and with this command:
 *
 *  $ hhvm -v Eval.EnableCodeCoverage=1 \
 *         -v Eval.CodeCoverageOutputFile=/tmp/cov.log \
 *         -f cover.php
 *
 * one get this output (with appropriate printf in this file):
 *
 *   /home/pad/cover.php, (1)
 *   /home/pad/cover.php, (3)
 *   here
 *   /home/pad/cover.php, (4)
 *   /home/pad/cover.php, (4)
 *   /home/pad/cover.php, (5)
 *   here 0
 *   /home/pad/cover.php, (4)
 *   /home/pad/cover.php, (4)
 *   /home/pad/cover.php, (5)
 *   here 1
 *   /home/pad/cover.php, (4)
 *   /home/pad/cover.php, (4)
 *   /home/pad/cover.php, (8)
 *   /home/pad/cover.php, (8)
 *   /home/pad/cover.php, (8)
 *   6/home/pad/cover.php, (12)
 *   /home/pad/cover.php, (12)
 *   /home/pad/cover.php, (1)
 *   /home/pad/cover.php, (1)
 *
 */
void CodeCoverage::Record(const char *filename, int line) {
  assertx(m_hits.has_value());
  if (!filename || !*filename || line <= 0 ) {
    return;
  }
  Logger::Verbose("%s, %d\n", filename, line);

  auto iter = m_hits->find(filename);
  if (iter == m_hits->end()) {
    auto& lines = (*m_hits)[filename];
    lines.resize(line + 1);
    lines.set(line);
  } else {
    auto& lines = iter->second;
    if ((int)lines.size() < line + 1) {
      lines.resize(line + 1);
    }
    lines.set(line);
  }
}

/*
 * Frequency reported is binary ie. either 0 or 1. This is not
 * changed to Array to make return type backwards compatible.
 */
Array CodeCoverage::Report(bool report_frequency /* = false*/,
                           bool sys /* = true */) {
  assertx(m_hits.has_value());
  Array ret = Array::CreateDict();
  for (const auto& iter : *m_hits) {
    if (!sys && Extension::IsSystemlibPath(iter.first)) {
      continue;
    }
    const auto& lines = iter.second;

    if (report_frequency) {
      auto count = lines.count();
      ret.set(String(iter.first), Variant((int64_t)count));
    } else {
      auto tmp = Array::CreateDict();
      auto const end = req::dynamic_bitset::npos;
      for (int i = lines.find_first(); i != end; i = lines.find_next(i)) {
        tmp.set(i, Variant((int64_t)1));
      }
      ret.set(String(iter.first), Variant(tmp));
    }
  }

  return ret;
}

void CodeCoverage::Report(const std::string &filename) {
  assertx(m_hits.has_value());
  std::ofstream f(filename.c_str());
  if (!f) {
    Logger::Error("unable to open %s", filename.c_str());
    return;
  }

  f << "{\n";
  for (CodeCoverageMap::const_iterator iter = m_hits->begin();
       iter != m_hits->end();) {
    f << "\"" << iter->first << "\": [";
    const auto& lines = iter->second;
    for (int i = 0; i != lines.size(); ) {
      f << (lines.test(i)? 1: 0);
      if (++i != lines.size()) {
        f << ",";
      }
    }
    f << "]";
    if (++iter != m_hits->end()) {
      f << ",";
    }
    f << "\n";
  }
  f << "}\n";

  f.close();
}

void CodeCoverage::Reset() {
  assertx(m_hits.has_value());
  m_hits->clear();
  resetCoverageCounters();
}

void CodeCoverage::onSessionInit() {
  m_hits.emplace();
}

void CodeCoverage::onSessionExit() {
  if (shouldDump) Report(Cfg::Eval::CodeCoverageOutputFile);
  shouldDump = false;
  m_hits.reset();
}

///////////////////////////////////////////////////////////////////////////////
}
