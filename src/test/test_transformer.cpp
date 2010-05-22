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

#include <test/test_transformer.h>
#include <compiler/parser/parser.h>
#include <compiler/code_generator.h>
#include <compiler/statement/statement_list.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/builtin_symbols.h>
#include <util/util.h>
#include <compiler/option.h>

using namespace std;

///////////////////////////////////////////////////////////////////////////////

TestTransformer::TestTransformer() : m_verbose(true) {
}

bool TestTransformer::VerifyTransformer(const char *input, const char *output,
                                        const char *file /* = "" */,
                                        int line /* = 0 */) {
  ASSERT(input);
  ASSERT(output);

  AnalysisResultPtr ar(new AnalysisResult());
  BuiltinSymbols::Load(ar);
  Parser::ParseString(input, ar);
  ar->analyzeProgram();
  ar->inferTypes();
  ostringstream code;
  CodeGenerator cg(&code);
  Option::GenerateCPPMacros = m_verbose;
  ar->outputAllCPP(cg);
  string actual = code.str();
  if (actual != output) {
    Util::replaceAll(actual, "\"", "\\\"");
    Util::replaceAll(actual, "\n", "\\n\"\n     \"");
    printf("-------------------------------------------------\n"
           "%s:%d\nParsing: [%s]\nExpecting %d: [%s]\nGot %d: [%s]\n",
           file, line, input, (int)strlen(output), output,
           (int)code.str().length(), actual.c_str());
    return false;
  }
  return true;
}
