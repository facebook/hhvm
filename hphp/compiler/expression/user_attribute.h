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

#ifndef incl_HPHP_USER_ATTRIBUTE_H_
#define incl_HPHP_USER_ATTRIBUTE_H_

#include "hphp/compiler/expression/expression.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(UserAttribute);

class UserAttribute : public Expression {
public:
  UserAttribute(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                const std::string& name,
                ExpressionPtr exp);

  DECLARE_BASE_EXPRESSION_VIRTUAL_FUNCTIONS;
  const std::string& getName() { return m_name;}
  ExpressionPtr getExp() { return m_exp;}
  virtual int getLocalEffects() const { return NoEffect; }

private:
  std::string m_name;
  ExpressionPtr m_exp;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_USER_ATTRIBUTE_H_
