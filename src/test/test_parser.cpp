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

#include <test/test_parser.h>
#include <compiler/parser/parser.h>
#include <compiler/code_generator.h>
#include <compiler/statement/statement_list.h>
#include <compiler/analysis/analysis_result.h>

using namespace std;

///////////////////////////////////////////////////////////////////////////////

bool TestParser::VerifyParser(const char *input, const char *output,
                              const char *file /* = "" */,
                              int line /* = 0 */) {
  ASSERT(input);
  ASSERT(output);

  AnalysisResultPtr ar(new AnalysisResult());
  StatementListPtr tree = Parser::ParseString(input, ar);
  ostringstream code;
  CodeGenerator cg(&code);
  tree->outputPHP(cg, ar);
  if (code.str() != output) {
    printf("%s:%d\nParsing: [%s]\nExpecting %d: [%s]\nGot %d: [%s]\n",
           file, line, input, (int)strlen(output), output,
           (int)code.str().length(), code.str().c_str());
    return false;
  }
  return true;
}
