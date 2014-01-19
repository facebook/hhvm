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

#ifndef incl_HPHP_ARRAY_ELEMENT_EXPRESSION_H_
#define incl_HPHP_ARRAY_ELEMENT_EXPRESSION_H_

#include "hphp/compiler/expression/expression.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ArrayElementExpression);

class ArrayElementExpression : public Expression,
                               public LocalEffectsContainer {
public:
  ArrayElementExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                         ExpressionPtr variable, ExpressionPtr offset);

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;
  DECL_AND_IMPL_LOCAL_EFFECTS_METHODS;

  ExpressionPtr preOptimize(AnalysisResultConstPtr ar);
  ExpressionPtr postOptimize(AnalysisResultConstPtr ar);

  virtual bool isRefable(bool checkError = false) const { return true;}
  bool isTemporary() const;

  ExpressionPtr getVariable() const { return m_variable;}
  ExpressionPtr getOffset() const { return m_offset;}
  virtual void setContext(Context context);
  virtual void clearContext(Context context);

  bool isSuperGlobal() const { return m_global;}
  bool isDynamicGlobal() const { return m_dynamicGlobal;}
  const std::string &getGlobalName() const { return m_globalName;}
  ExpressionPtr unneeded();

  /**
   * This is purely for resolving a nasty case of intepreting
   * self::$a[1][2] correctly.
   */
  bool appendClass(ExpressionPtr cls,
                   AnalysisResultConstPtr ar, FileScopePtr file);

  virtual bool canonCompare(ExpressionPtr e) const;

private:
  ExpressionPtr m_variable;
  ExpressionPtr m_offset;
  bool m_global;
  bool m_dynamicGlobal;
  std::string m_globalName;
  std::string m_text;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ARRAY_ELEMENT_EXPRESSION_H_
