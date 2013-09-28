/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/test/ext/test_code_run.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/compiler/builtin_symbols.h"
#include "hphp/compiler/code_generator.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/type.h"
#include "hphp/util/util.h"
#include "hphp/util/process.h"
#include "hphp/compiler/option.h"
#include "hphp/runtime/base/runtime-option.h"
#include <pcre.h>
#include "hphp/runtime/ext/ext_file.h"

using std::istringstream;
using std::ostringstream;

///////////////////////////////////////////////////////////////////////////////

TestCodeRun::TestCodeRun() : m_perfMode(false), m_test(0) {
  Option::KeepStatementsWithNoEffect = false;
  Option::ParserThreadCount = 4;
}

bool TestCodeRun::preTest() {
  if (!CleanUp()) return false;
  return true;
}

bool TestCodeRun::postTest() {
  return true;
}

bool TestCodeRun::CleanUp() {
  string out, err;
  const char *argv[] = {"", nullptr};
  Process::Exec("runtime/tmp/cleanup.sh", argv, nullptr, out, &err);
  if (!err.empty()) {
    printf("Failed to clean up runtime/tmp: %s\n", err.c_str());
    return false;
  }
  return true;
}

static bool GenerateMainPHP(const std::string &fullPath,
                            const char *file, int line,
                            const char *input) {
  Util::mkdir(fullPath.c_str());
  std::ofstream f(fullPath.c_str());
  if (!f) {
    printf("Unable to open %s for write. Run this test from hphp/.\n",
           fullPath.c_str());
    return false;
  }

  if (file && !strncmp(input, "<?php", 5)) {
    f << "<?php\n# " << file << ":" << line << "\n";
    f << (input + 5);
  } else {
    f << input;
  }
  f.close();
  return true;
}

static string escape(const std::string &s) {
  string ret;
  ret.reserve(s.size() + 20);
  for (unsigned int i = 0; i < s.length(); i++) {
    char ch = s[i];
    if (isprint(ch) || ch == '\n') {
      ret += ch;
    } else {
      char buf[10];
      snprintf(buf, sizeof(buf), "{\\x%02X}", (unsigned char)ch);
      ret += buf;
    }
  }
  return ret;
}

static bool verify_result(const char *input, const char *output, bool perfMode,
                          const char *file = "", int line = 0,
                          bool nowarnings = false,
                          bool fileoutput = false,
                          const char *subdir = "",
                          bool fastMode = false) {
  // generate main.php
  string fullPath = "runtime/tmp";
  if (subdir && subdir[0]) fullPath = fullPath + "/" + subdir;
  fullPath += "/main.php";
  if (!GenerateMainPHP(fullPath, 0, 0, input)) return false;

  // get PHP's output if "output" is NULL
  string expected;
  if (output) {
    if (fileoutput) {
      String s = f_file_get_contents(output);
      expected = string(s.data(), s.size());
    } else {
      expected = output;
    }
  } else {
    const char *argv1[] = {"", "-ddisplay_errors=stderr",
                           "-dapc.enable_cli=1",
                           fullPath.c_str(), nullptr};
    const char *argv2[] = {"", "-ddisplay_errors=On",
                           "-dapc.enable_cli=1",
                           fullPath.c_str(), nullptr};
    string err;
    Process::Exec(php_path, nowarnings ? argv2 : argv1, nullptr, expected, &err);
    if (!err.empty() && nowarnings) {
      printf("%s:%d\nParsing: [%s]\nFailed to run %s: %s\n",
             file, line, input, fullPath.c_str(), err.c_str());
      return false;
    }
  }

  // run and verify output
  {
    string actual, err;
    string dir = "runtime/tmp/";
    if (subdir) dir = dir + subdir + "/";
    string repoarg = "-vRepo.Central.Path=" + dir + "hhvm.hhbc";

    if (Option::EnableEval < Option::FullEval) {
      if (fastMode) {
        string path = dir + "libtest.so";
        const char *argv[] = {"", "--file=string", "--config=test/slow/config.hdf",
                              repoarg.c_str(), path.c_str(), nullptr};
        Process::Exec("runtime/tmp/run.sh", argv, nullptr, actual, &err);
      } else {
        const char *argv[] = {"", "--file=string", "--config=test/slow/config.hdf",
                              repoarg.c_str(), nullptr};
        string path = dir + "test";
        Process::Exec(path.c_str(), argv, nullptr, actual, &err);
      }
    } else {
      string filearg = "--file=" + dir + "main.php";

      string jitarg = string("-vEval.Jit=") +
        (RuntimeOption::EvalJit ? "true" : "false");
      const char *argv[] = {"", filearg.c_str(),
                            "--config=test/slow/config.hdf",
                            repoarg.c_str(),
                            jitarg.c_str(),
                            nullptr};
      Process::Exec(HHVM_PATH, argv, nullptr, actual, &err);
    }

    if (perfMode) {
      string sinput = input;
      const char *marker = "/* INPUT */";
      int pos1 = sinput.find(marker);
      int pos2 = sinput.find(marker, pos1+1);
      pos1 += strlen(marker);
      sinput = sinput.substr(pos1, pos2 - pos1);
      if (sinput.size() > 1000) sinput = "(long program)";

      // we have to adjust timing by removing loop cost, which is the 1st test
      static int adj1 = -1;
      static int adj2 = -1;
      int ms1 = atoi(expected.c_str());
      int ms2 = atoi(actual.c_str());
      if (adj1 == -1) adj1 = ms1;
      if (adj2 == -1) adj2 = ms2;
      int msAdj1 = ms1 - adj1;
      int msAdj2 = ms2 - adj2;
      double x = 0.0; // how many times faster
      double p = 0.0; // percentage
      if (msAdj2 != 0) {
        x = ((double)(int)(msAdj1 * 100 / msAdj2)) / 100;
      }
      if (msAdj1 != 0) {
        p = ((double)(int)(msAdj2 * 10000 / msAdj1)) / 100;
      }

      printf("----------------------------------------------------------\n"
             "%s\n\n"
             "        PHP        C++\n"
             "===========================================\n"
             "  %6d ms  %6d ms\n"
             " -%6d ms  %6d ms\n"
             "===========================================\n"
             "  %6d ms  %6d ms   =   %2.4gx  or  %2.4g%%\n\n",
             sinput.c_str(), ms1, ms2, adj1, adj2, msAdj1, msAdj2, x, p);
      return true;
    }

    bool out_ok = actual == expected;
    if (!out_ok || (!nowarnings && !err.empty())) {
      if (out_ok &&
          err.find("symbol lookup error:") != string::npos &&
          err.find("undefined symbol: ") != string::npos) {
        printf("%s: Ignoring loader error: %s\n",
               fullPath.c_str(), err.c_str());
      } else {
        printf("======================================\n"
               "%s:\n"
               "======================================\n"
               "%s:%d\nParsing: [%s]\nBet %d:\n"
               "--------------------------------------\n"
               "%s"
               "--------------------------------------\n"
               "Got %d:\n"
               "--------------------------------------\n"
               "%s"
               "--------------------------------------\n"
               "Err: [%s]\n", fullPath.c_str(), file, line, input,
               (int)expected.length(), escape(expected).c_str(),
               (int)actual.length(), escape(actual).c_str(),
               err.c_str());
        return false;
      }
    }
  }

  return true;
}

bool TestCodeRun::RecordMulti(const char *input, const char *output,
                              const char *file, int line, bool nowarnings,
                              bool fileoutput) {
  string fullPath = "runtime/tmp/" + Test::s_suite + "/" + test_name + "/tcr-" +
    boost::lexical_cast<string>(m_test++);

  if (!GenerateMainPHP(fullPath + "/main.php", file, line, input)) return false;
  if (nowarnings) {
    std::ofstream((fullPath + "/nowarnings").c_str());
  }

  if (output) {
    std::ofstream s((fullPath + "/test.result").c_str());
    if (fileoutput) {
      String expected = f_file_get_contents(output);
      s << string(expected.data(), expected.size());
    } else {
      s << output;
    }
  }

  return true;
}

bool TestCodeRun::VerifyCodeRun(const char *input, const char *output,
                                const char *file /* = "" */,
                                int line /* = 0 */,
                                bool nowarnings /* = false */,
                                bool fileoutput /* = false */) {
  assert(input);
  if (!CleanUp()) return false;

  return verify_result(input, output, m_perfMode,
                       file, line, nowarnings, fileoutput, "Test0", false);
}
