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
#include "hphp/runtime/base/code-coverage.h"

#include <fstream>
#include <vector>

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"

#include "hphp/util/logger.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * The function below will be called by the interpreter on each
 * evaluated expression when running hhvm with:
 *   $ hhvm -v Eval.RecordCodeCoverage=1 <phpfile>
 *
 * The (line0, line1) pair is supposed to represent the start and end
 * of an evaluated expression. One should normally increment the line
 * usage counters for all the lines between line0 and line1 but certain
 * constructs like including a file, eval-ing a string, or even
 * executing a for loop do not have a good value for line1.
 *
 * Indeed on this toy cover.php file:
 *
 *  <?php
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
 *  $ hhvm -v Eval.RecordCodeCoverage=1 \
 *         -v Eval.CodeCoverageOutputFile=/tmp/cov.log \
 *         -f cover.php
 *
 * one get this output (with appropriate printf in this file):
 *
 *   /home/pad/cover.php, (1, 12)
 *   /home/pad/cover.php, (3, 3)
 *   here
 *   /home/pad/cover.php, (4, 6)
 *   /home/pad/cover.php, (4, 4)
 *   /home/pad/cover.php, (5, 5)
 *   here 0
 *   /home/pad/cover.php, (4, 4)
 *   /home/pad/cover.php, (4, 4)
 *   /home/pad/cover.php, (5, 5)
 *   here 1
 *   /home/pad/cover.php, (4, 4)
 *   /home/pad/cover.php, (4, 4)
 *   /home/pad/cover.php, (8, 10)
 *   /home/pad/cover.php, (8, 9)
 *   /home/pad/cover.php, (8, 10)
 *   6/home/pad/cover.php, (12, 12)
 *   /home/pad/cover.php, (12, 12)
 *   /home/pad/cover.php, (1, 2)
 *   /home/pad/cover.php, (1, 2)
 *
 * So right now it is just simpler to ignore line1.
 */
void CodeCoverage::Record(const char *filename, int line0, int line1) {
  if (!filename || !*filename || line0 <= 0 || line1 <= 0 || line0 > line1) {
    return;
  }
  Logger::Verbose("%s, (%d, %d)\n", filename, line0, line1);

  CodeCoverageMap::iterator iter = m_hits.find(filename);
  if (iter == m_hits.end()) {
    std::vector<int> &lines = m_hits[filename];
    lines.resize(line1 + 1);
    for (int i = line0; i <= line0 /* should be line1 one day */; i++) {
      lines[i] = 1;
    }
  } else {
    std::vector<int> &lines = iter->second;
    if ((int)lines.size() < line1 + 1) {
      lines.resize(line1 + 1);
    }
    for (int i = line0; i <= line0 /* should be line1 one day */; i++) {
      ++lines[i];
    }
  }
}

Array CodeCoverage::Report() {
  Array ret = Array::Create();
  for (CodeCoverageMap::const_iterator iter = m_hits.begin();
       iter != m_hits.end(); ++iter) {
    const std::vector<int> &lines = iter->second;
    Array tmp = Array::Create();
    for (int i = 1; i < (int)lines.size(); i++) {
      if (lines[i]) {
        tmp.set(i, Variant((int64_t)lines[i]));
      }
    }
    ret.set(String(iter->first), Variant(tmp));
  }

  return ret;
}

void CodeCoverage::Report(const std::string &filename) {
  std::ofstream f(filename.c_str());
  if (!f) {
    Logger::Error("unable to open %s", filename.c_str());
    return;
  }

  f << "{\n";
  for (CodeCoverageMap::const_iterator iter = m_hits.begin();
       iter != m_hits.end();) {
    const std::vector<int> &lines = iter->second;
    f << "\"" << iter->first << "\": [";
    int count = lines.size();
    for (int i = 0 /* not 1 */; i < count; i++) {
      f << lines[i];
      if (i < count - 1) {
        f << ",";
      }
    }
    f << "]";
    if (++iter != m_hits.end()) {
      f << ",";
    }
    f << "\n";
  }
  f << "}\n";

  f.close();
}

void CodeCoverage::Reset() {
  m_hits.clear();
  g_context->resetCoverageCounters();
}

///////////////////////////////////////////////////////////////////////////////
}

