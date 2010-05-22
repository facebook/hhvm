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

#include <runtime/eval/ast/simple_function_call_expression.h>
#include <runtime/eval/ast/name.h>
#include <runtime/base/string_util.h>
#include <runtime/eval/ast/scalar_expression.h>
#include <runtime/eval/ast/class_statement.h>
#include <runtime/eval/runtime/eval_state.h>
#include <runtime/eval/ast/function_statement.h>
#include <runtime/eval/parser/parser.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

SimpleFunctionCallExpression::SimpleFunctionCallExpression
(EXPRESSION_ARGS, NamePtr name, const std::vector<ExpressionPtr> &params) :
  FunctionCallExpression(EXPRESSION_PASS, params), m_name(name) {}

Variant SimpleFunctionCallExpression::eval(VariableEnvironment &env) const {
  SET_LINE;
  String name(m_name->get(env));
  bool renamed = false;
  {
    // so hacky, gotta do this properly by overriding rename_function.
    hphp_const_char_imap<const char*> &funcs = get_renamed_functions();
    hphp_const_char_imap<const char*>::const_iterator iter =
      funcs.find(name.data());
    if (iter != funcs.end()) {
      name = iter->second;
      renamed = true;
    }
  }
  // fast path for interpreted fn
  const Function *fs = RequestEvalState::findFunction(name.c_str());
  if (fs) {
    return ref(fs->directInvoke(env, this));
  } else {
    return ref(invoke_from_eval(name.data(), env, this,
                                renamed ? -1 : m_name->hashLwr()));
  }
}

void SimpleFunctionCallExpression::dump() const {
  m_name->dump();
  dumpParams();
}

ExpressionPtr
SimpleFunctionCallExpression::make(EXPRESSION_ARGS, NamePtr name,
                                   const vector<ExpressionPtr> &params,
                                   const Parser &p) {
  String sname = StringUtil::ToLower(name->getStatic());
  if (!sname.isNull()) {
    if (sname == "get_class" && params.size() == 0) {
      if (p.currentClass()) {
        return new ScalarExpression(EXPRESSION_PASS, p.currentClass()->name());
      } else {
        return new ScalarExpression(EXPRESSION_PASS, false);
      }
    } else if (sname == "get_parent_class" && params.size() == 0) {
      if (p.currentClass() && !p.currentClass()->parent().empty()) {
        return new ScalarExpression(EXPRESSION_PASS,
                                    p.currentClass()->parent());
      } else {
        return new ScalarExpression(EXPRESSION_PASS, false);
      }
    }
  }
  return new SimpleFunctionCallExpression(EXPRESSION_PASS, name, params);
}

///////////////////////////////////////////////////////////////////////////////
}
}

