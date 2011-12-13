/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __EVAL_TRAIT_PREC_STATEMENT_H__
#define __EVAL_TRAIT_PREC_STATEMENT_H__

#include <runtime/eval/ast/expression.h>
#include <runtime/eval/ast/name.h>
#include <runtime/eval/ast/statement.h>
#include <runtime/eval/ast/statement_list_statement.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(TraitPrecStatement);

class TraitPrecStatement : public Statement {
public:
  TraitPrecStatement(STATEMENT_ARGS, NamePtr traitName, NamePtr methodName,
                     const std::vector<NamePtr> &names);
  Name *getTraitName() const { return m_traitName.get(); }
  Name *getMethodName() const { return m_methodName.get(); }
  const std::vector<NamePtr> &getNames() const { return m_names; }

  virtual void eval(VariableEnvironment &env) const;
  virtual void dump(std::ostream &out) const;
private:
  NamePtr m_traitName;
  NamePtr m_methodName;
  std::vector<NamePtr> m_names;
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_TRAIT_PREC_STATEMENT_H__ */
