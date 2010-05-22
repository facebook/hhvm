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

#ifndef __CONSTANT_EXPRESSION_H__
#define __CONSTANT_EXPRESSION_H__

#include <compiler/expression/expression.h>

#define CONSTANT(value)                                 \
  (Expression::makeConstant(ar, getLocation(), value))

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ConstantExpression);

class ConstantExpression : public Expression {
public:
  ConstantExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                     const std::string &name);

  DECLARE_BASE_EXPRESSION_VIRTUAL_FUNCTIONS;
  virtual bool isScalar() const;
  virtual int getLocalEffects() const { return NoEffect; }
  virtual bool getScalarValue(Variant &value);
  virtual bool containsDynamicConstant(AnalysisResultPtr ar) const {
    return m_dynamic;
  }

  virtual unsigned getCanonHash() const;
  virtual bool canonCompare(ExpressionPtr e) const;

  const std::string &getName() const { return m_name;}
  bool isBoolean() const;
  bool isNull() const;
  bool getBooleanValue() const;
  void pushConst(const std::string &name);
  void popConst();
  void setComment(const std::string comment) { m_comment = comment;}
  const std::string getComment() { return m_comment;}

private:
  std::string m_name;
  bool m_valid;
  bool m_dynamic;
  bool m_visited;
  std::string m_comment; // for inlined constant name
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __CONSTANT_EXPRESSION_H__
