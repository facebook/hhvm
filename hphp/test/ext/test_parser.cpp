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

#include "hphp/test/ext/test_parser.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/compiler/code_generator.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/util/util.h"

///////////////////////////////////////////////////////////////////////////////

static void strip_empty_block(std::string &s) {
  if (s.size() >= 2 && s[0] == '{' && s[s.size() - 1] == '}') {
    s = s.substr(1, s.size() - 2);
  }
}

bool TestParser::SameCode(std::string code1, std::string code2) {
  Util::replaceAll(code1, "\n", "");
  Util::replaceAll(code2, "\n", "");

  Util::replaceAll(code1, "{{}}", "{}");
  Util::replaceAll(code2, "{{}}", "{}");
  Util::replaceAll(code1, "else {}", "");
  Util::replaceAll(code2, "else {}", "");
  strip_empty_block(code1);
  strip_empty_block(code2);

  return code1 == code2;
}

bool TestParser::VerifyParser(const char *input, const char *output,
                              const char *file /* = "" */, int line /* = 0 */,
                              const char *output2 /* = NULL */) {
  assert(input);
  assert(output);
  if (output2 == nullptr) output2 = output;

  string oldTab = Option::Tab;
  Option::Tab = "";

  bool ret = true;
  {
    AnalysisResultPtr ar(new AnalysisResult());
    StatementListPtr tree = Compiler::Parser::ParseString(input, ar);
    std::ostringstream code;
    CodeGenerator cg(&code);
    tree->outputPHP(cg, ar);
    if (!SameCode(code.str(), output)) {
      printf("======================================\n"
             "[Compiler] %s:%d:\n"
             "======================================\n",
             file, line);
      printf("[%s]\nExpecting %d: [%s]\nGot %d: [%s]\n",
             input, (int)strlen(output), output,
             (int)code.str().length(), code.str().c_str());
      ret = false;
    }
  }

  Option::Tab = oldTab;
  return ret;
}
