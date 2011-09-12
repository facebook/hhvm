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

#include <runtime/eval/ast/scalar_value_expression.h>
#include <runtime/eval/ast/variable_expression.h>
#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/base/variable_serializer.h>
#include <runtime/base/variable_serializer.h>
#include <runtime/base/runtime_option.h>
#include <tbb/concurrent_hash_map.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_THREAD_LOCAL(ScalarValueExpression::ScalarValueExpressionSet,
                       ScalarValueExpression::s_scalarValueExpressions);

IMPLEMENT_THREAD_LOCAL(ScalarValueExpression::ScalarValueExpressionRefSet,
                       ScalarValueExpression::s_refs);

typedef tbb::concurrent_hash_map<std::string, ScalarValueExpression *,
                                 stringHashCompare> ScalarValueExpressionMap;
static ScalarValueExpressionMap s_scalarValueExpressionMap;

void ScalarValueExpression::InsertExpressionPtr(ExpressionPtr *astPtr) {
  s_refs->insert(astPtr);
}

void ScalarValueExpression::RemoveExpressionPtr(ExpressionPtr *astPtr) {
  s_refs->erase(astPtr);
}

void register_for_scalar_value_expression(void *astPtr, bool insert) {
  if (insert) {
    ScalarValueExpression::InsertExpressionPtr((ExpressionPtr *)(astPtr));
  } else {
    ScalarValueExpression::RemoveExpressionPtr((ExpressionPtr *)(astPtr));
  }
}

ScalarValueExpression *ScalarValueExpression::GetScalarValueExpression(
  ScalarValueExpression *exp) {
  String s = f_serialize(exp->m_value);
  if (s.size() >= RuntimeOption::EvalScalarValueExprLimit) return NULL;
  std::string key(s.data(), s.size());
  ScalarValueExpressionMap::accessor acc;
  if (s_scalarValueExpressionMap.insert(acc, key)) {
    acc->second = exp;
    acc->second->setStatic();
    acc->second->m_value.setEvalScalar();
    acc->second->m_value.setVarNR();
  }
  return acc->second;
}

ScalarValueExpression::ScalarValueExpression(CVarRef value,
  const Location* loc) : Expression(KindOfScalarValueExpression, loc),
  m_value(value) {
  s_scalarValueExpressions->insert(this);
}

ScalarValueExpression::~ScalarValueExpression() {
  s_scalarValueExpressions->erase(this);
}

void ScalarValueExpression::dump(std::ostream &out) const {
  VariableSerializer vs(VariableSerializer::Serialize);
  Variant ret(vs.serialize(m_value, true));
  out << ret.toString().data();
}

void ScalarValueExpression::initScalarValues() {
  s_scalarValueExpressions->clear();
}

void ScalarValueExpression::registerScalarValues() {
  std::map<ScalarValueExpression *, std::vector<ExpressionPtr *> > reverseMap;
  for (ScalarValueExpressionRefSet::const_iterator it = s_refs->begin();
       it != s_refs->end(); it++) {
    ExpressionPtr *ptr = (*it);
    Expression *exp = ptr->get();
    ASSERT(exp->isKindOf(KindOfScalarValueExpression));
    reverseMap[static_cast<ScalarValueExpression *>(exp)].push_back(ptr);
  }
  ScalarValueExpressionSet tmpSet(*s_scalarValueExpressions);
  for (ScalarValueExpressionSet::iterator it = tmpSet.begin();
       it != tmpSet.end(); it++) {
    ScalarValueExpression *p = (*it);
    unsigned int count = p->getCount();
    ScalarValueExpression *sp = GetScalarValueExpression(p);
    if (sp) {
      std::map<ScalarValueExpression *, std::vector<ExpressionPtr *> >::
        iterator it2 = reverseMap.find(p);
      if (it2 != reverseMap.end()) {
        assert(count == it2->second.size());
        for (unsigned int i = 0; i < it2->second.size(); i++) {
          *it2->second[i] = sp;
        }
      } else {
        ASSERT(false);
      }
    } else {
      p->m_value.setEvalScalar();
      p->m_value.setVarNR();
    }
  }
  s_scalarValueExpressions->clear();
  s_refs->clear();
}

///////////////////////////////////////////////////////////////////////////////
}
}

