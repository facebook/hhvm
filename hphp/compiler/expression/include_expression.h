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

#ifndef incl_HPHP_INCLUDE_EXPRESSION_H_
#define incl_HPHP_INCLUDE_EXPRESSION_H_

#include "hphp/compiler/expression/unary_op_expression.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(IncludeExpression);

class IncludeExpression : public UnaryOpExpression, public IParseHandler {
public:
  static std::string CheckInclude(ConstructPtr includeExp,
                                  FileScopePtr scope,
                                  ExpressionPtr fileExp,
                                  bool &documentRoot);

public:
  IncludeExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                    ExpressionPtr exp, int op);

  DECLARE_BASE_EXPRESSION_VIRTUAL_FUNCTIONS;
  ExpressionPtr preOptimize(AnalysisResultConstPtr ar) override;

  // implementing IParseHandler
  void onParse(AnalysisResultConstPtr ar, FileScopePtr scope) override;

  bool isReqLit() const;
  void setDocumentRoot() { m_documentRoot = true;}
  bool isDocumentRoot() const { return m_documentRoot;}
  std::string includePath() const;
  FileScopeRawPtr getIncludedFile(AnalysisResultConstPtr) const;
private:
  /**
   * There are 3 forms of include paths:
   *
   *   1. "/<absolute_path>" starts with '/'.
   *   2. "<relative_path>" starts without '/': relative to containing file
   *   3. "<relative_path>" + m_documentRoot == true: relative to doc root
   *
   * privateScope means the include gets its own variable environment
   * privateInclude means this is the *only* reference to the included file
   */
  unsigned m_documentRoot : 1;
  unsigned m_depsSet : 1;
  std::string m_include;

  bool analyzeInclude(AnalysisResultConstPtr ar, const std::string &include);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_INCLUDE_EXPRESSION_H_
