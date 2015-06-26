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

#ifndef incl_HPHP_TRAIT_PREC_STATEMENT_H_
#define incl_HPHP_TRAIT_PREC_STATEMENT_H_

#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/statement/statement.h"

#include <unordered_set>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ExpressionList);
DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(TraitPrecStatement);

class TraitPrecStatement : public Statement {
public:
  TraitPrecStatement(STATEMENT_CONSTRUCTOR_PARAMETERS,
                     ScalarExpressionPtr className,
                     ScalarExpressionPtr methodName,
                     ExpressionListPtr   otherClassNames);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;
  const std::string& getMethodName() const { return m_methodName->getString(); }
  const std::string& getTraitName()  const { return m_traitName->getString(); }
  void getOtherTraitNames(
    hphp_string_iset& nameSet) const;

private:
  ScalarExpressionPtr m_traitName;        // selected trait name
  ScalarExpressionPtr m_methodName;
  ExpressionListPtr   m_otherTraitNames;  // non-selected trait names
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_TRAIT_PREC_STATEMENT_H_
