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

#ifndef incl_HPHP_CLASS_CONSTANT_EXPRESSION_H_
#define incl_HPHP_CLASS_CONSTANT_EXPRESSION_H_

#include "hphp/compiler/expression/static_class_name.h"
#include "hphp/compiler/analysis/block_scope.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ClassConstantExpression);

class ClassConstantExpression : public Expression, public StaticClassName {
public:
  ClassConstantExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                          ExpressionPtr classExp,
                          const std::string &varName);

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;
  ExpressionPtr preOptimize(AnalysisResultConstPtr ar);
  virtual int getLocalEffects() const { return NoEffect; }

  virtual unsigned getCanonHash() const;
  virtual bool canonCompare(ExpressionPtr e) const;

  virtual bool containsDynamicConstant(AnalysisResultPtr ar) const;

  const std::string &getConName() const { return m_varName; }
  std::string getActualClassName() const;

  bool isValid() const { return m_valid; }
  bool isDynamic() const;
  bool hasClass() const { return m_defScope != 0; }
private:
  std::string m_varName;
  BlockScope *m_defScope;
  bool m_valid;
  bool m_depsSet;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_CLASS_CONSTANT_EXPRESSION_H_
