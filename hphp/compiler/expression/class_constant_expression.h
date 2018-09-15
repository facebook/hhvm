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

#ifndef incl_HPHP_CLASS_CONSTANT_EXPRESSION_H_
#define incl_HPHP_CLASS_CONSTANT_EXPRESSION_H_

#include "hphp/compiler/expression/static_class_name.h"
#include "hphp/compiler/analysis/block_scope.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ClassConstantExpression);

struct ClassConstantExpression : Expression, StaticClassName {
  ClassConstantExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                          ExpressionPtr classExp,
                          const std::string &varName);

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;
  void analyzeProgram(AnalysisResultConstRawPtr ar) override;

  const std::string &getConName() const { return m_varName; }

  ClassScopeRawPtr getOriginalClassScope() const;

  bool hasClass() const = delete;
  bool isColonColonClass() const {
    return (m_varName.size() == strlen("class")) &&
           !strcasecmp(m_varName.c_str(), "class");
  }
private:
  std::string m_varName;
  bool m_originalScopeSet;
  BlockScopeRawPtr m_originalScope;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_CLASS_CONSTANT_EXPRESSION_H_
