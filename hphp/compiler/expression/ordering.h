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

#ifndef incl_HPHP_ORDERING_H_
#define incl_HPHP_ORDERING_H_

#include "hphp/compiler/expression/expression.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/parser/scanner.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(Ordering);

class Ordering : public Expression {

public:
  Ordering(EXPRESSION_CONSTRUCTOR_PARAMETERS,
    ExpressionPtr key, TokenID direction);

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;

  ExpressionPtr getKey() const { return m_key; }
  TokenID getDirection() { return m_direction; }
private:
  ExpressionPtr m_key;
  TokenID m_direction;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ORDERING_H_
