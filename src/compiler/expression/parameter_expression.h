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

#ifndef __PARAMETER_EXPRESSION_H__
#define __PARAMETER_EXPRESSION_H__

#include <compiler/expression/expression.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(Type);
DECLARE_BOOST_TYPES(AnalysisResult);
DECLARE_BOOST_TYPES(ParameterExpression);

class ParameterExpression : public Expression {
public:
  ParameterExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                      const std::string &type, const std::string &name,
                      bool ref, ExpressionPtr defaultValue);

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;

  bool isRef() const { return m_ref;}
  bool isOptional() const { return m_defaultValue;}
  bool hasRTTI() const { return m_hasRTTI;}
  void setHasRTTI() { m_hasRTTI = true;}
  const std::string &getName() const { return m_name;}
  void defaultToNull(AnalysisResultPtr ar);

  void rename(const std::string &name) { m_name = name;}
  ExpressionPtr defaultValue() { return m_defaultValue; }
private:
  std::string m_type;
  std::string m_name;
  bool m_ref;
  bool m_hasRTTI;
  ExpressionPtr m_defaultValue;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __PARAMETER_EXPRESSION_H__
