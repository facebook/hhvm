/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/eval/runtime/code_coverage.h>
#include <runtime/base/complex_types.h>
#include <util/logger.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

Mutex CodeCoverage::s_mutex;
CodeCoverage::CodeCoverageMap CodeCoverage::s_hits;

void CodeCoverage::Record(const char *filename, int line0, int line1) {
  if (!filename || !*filename || line0 <= 0 || line1 <= 0 || line0 > line1) {
    return;
  }

  Lock lock(s_mutex);

  CodeCoverageMap::iterator iter = s_hits.find(filename);
  if (iter == s_hits.end()) {
    vector<int> &lines = s_hits[filename];
    lines.resize(line1 + 1);
    for (int i = line0; i <= line1; i++) {
      lines[i] = 1;
    }
  } else {
    vector<int> &lines = iter->second;
    if ((int)lines.size() < line1 + 1) {
      lines.resize(line1 + 1);
    }
    for (int i = line0; i <= line1; i++) {
      ++lines[i];
    }
  }
}

Array CodeCoverage::Report() {
  Lock lock(s_mutex);

  Array ret = Array::Create();
  for (CodeCoverageMap::const_iterator iter = s_hits.begin();
       iter != s_hits.end(); ++iter) {
    const vector<int> &lines = iter->second;
    Array tmp = Array::Create();
    for (int i = 1; i < (int)lines.size(); i++) {
      if (lines[i]) {
        tmp.set(i, Variant((int64)lines[i]));
      }
    }
    ret.set(String(iter->first), Variant(tmp));
  }

  return ret;
}

void CodeCoverage::Report(const std::string &filename) {
  Lock lock(s_mutex);

  ofstream f(filename.c_str());
  if (!f) {
    HPHPLOG_ERROR("unable to open %s", filename.c_str());
    return;
  }

  f << "{\n";
  for (CodeCoverageMap::const_iterator iter = s_hits.begin();
       iter != s_hits.end();) {
    const vector<int> &lines = iter->second;
    f << "\"" << iter->first << "\": [";
    int count = lines.size();
    for (int i = 0 /* not 1 */; i < count; i++) {
      f << lines[i];
      if (i < count - 1) {
        f << ",";
      }
    }
    f << "]";
    if (++iter != s_hits.end()) {
      f << ",";
    }
    f << "\n";
  }
  f << "}\n";

  f.close();
}

void CodeCoverage::Reset() {
  Lock lock(s_mutex);
  s_hits.clear();
}

///////////////////////////////////////////////////////////////////////////////
}}

