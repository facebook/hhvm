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

#include <runtime/eval/ast/closure_expression.h>
#include <runtime/eval/ast/function_statement.h>
#include <runtime/eval/runtime/variable_environment.h>

#include <runtime/ext/ext_closure.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

ClosureExpression::ClosureExpression(EXPRESSION_ARGS,
                                     FunctionStatementPtr func,
                                     const vector<ParameterPtr> &vars)
    : Expression(KindOfClosureExpression, EXPRESSION_PASS), m_func(func) {
  func->setClosure(this);

  // push the vars in reverse order, not retaining duplicates
  set<string> seenBefore;
  for (vector<ParameterPtr>::const_reverse_iterator it(vars.rbegin());
       it != vars.rend();
       it++) {
    ParameterPtr param(*it);
    if (seenBefore.find(param->getName().c_str()) == seenBefore.end()) {
      seenBefore.insert(param->getName().c_str());
      m_vars.push_back(param);
    }
  }
  reverse(m_vars.begin(), m_vars.end());
}

Variant ClosureExpression::eval(VariableEnvironment &env) const {
  m_func->eval(env);
  Array vars;
  for (unsigned int i = 0; i < m_vars.size(); i++) {
    Parameter *param = m_vars[i].get();
    String name = param->getName();
    SuperGlobal sg;
    if (!param->getSuperGlobal(sg)) {
      sg = VariableIndex::isSuperGlobal(name);
    }
    if (param->isRef()) {
      vars.append(ref(env.getVar(name, sg)));
    } else {
      vars.append(env.getVar(name, sg));
    }
  }

  // need to get at low level constructor
  return Object(NEWOBJ(c_GeneratorClosure)(
                m_func->getClosureCallInfo(), m_func.get(), vars));
}

void ClosureExpression::dump(std::ostream &out) const {
  m_func->dumpHeader(out);
  if (!m_vars.empty()) {
    out << " use (";
    dumpVector(out, m_vars);
    out << ")";
  }
  m_func->dumpBody(out);
}

///////////////////////////////////////////////////////////////////////////////
}
}

