/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_USE_DECLARATION_STATEMENT_FRAGMENT_H_
#define incl_HPHP_USE_DECLARATION_STATEMENT_FRAGMENT_H_

#include "hphp/compiler/parser/parser.h"
#include "hphp/compiler/statement/statement.h"

namespace HPHP {

DECLARE_BOOST_TYPES(UseDeclarationStatementFragment);

/**
 * This is pretty nasty: it's not really a statement, it's morally more of an
 * std::pair. But the only way to pass data out of a parser production is via a
 * Token, and a Token can only have data in the form of an Expression,
 * Statement, or TypeAnnotation, so here we are.
 */
struct UseDeclarationStatementFragment : public Statement {
  std::string ns;
  std::string as;
  Compiler::Parser::UseDeclarationConsumer mixed_consumer;

  UseDeclarationStatementFragment(STATEMENT_CONSTRUCTOR_PARAMETERS,
                                  std::string ns, std::string as)
    : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(
          UseDeclarationStatementFragment)),
      ns(ns), as(as), mixed_consumer(nullptr) {
  }

  void analyzeProgram(AnalysisResultPtr ar) override {
    always_assert(false);
  }

  int getKidCount() const override {
    always_assert(false);
  }

  void outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) override {
    always_assert(false);
  }
};

}

#endif
