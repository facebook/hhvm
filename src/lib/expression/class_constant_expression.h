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

#ifndef __CLASS_CONSTANT_EXPRESSION_H__
#define __CLASS_CONSTANT_EXPRESSION_H__

#include <lib/expression/expression.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ClassConstantExpression);

class ClassConstantExpression : public Expression {
public:
  ClassConstantExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                          const std::string &className,
                          const std::string &varName);

  DECLARE_BASE_EXPRESSION_VIRTUAL_FUNCTIONS;
  virtual bool hasEffect() const { return false;}

private:
  std::string m_className;
  std::string m_varName;
  bool m_valid;
  bool m_redeclared;
  bool m_visited;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __CLASS_CONSTANT_EXPRESSION_H__
