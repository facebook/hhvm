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

#include <runtime/eval/analysis/block.h>
#include <runtime/eval/ast/static_statement.h>
#include <runtime/eval/ast/expression.h>
#include <runtime/eval/ast/name.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

void VariableIndex::set(CStrRef name, int idx) {
  m_idx = idx;
  m_hash = hash_string(name.c_str(), name.size());
  m_sg = isSuperGlobal(name);
}

VariableIndex::SuperGlobal VariableIndex::isSuperGlobal(CStrRef name) {
  if (name == "GLOBALS") {
    return Globals;
  } else if (name.data()[0] == '_') {
    if (name == "_SERVER") {
      return Server;
    } else if (name == "_GET") {
      return Get;
    } else if (name == "_POST") {
      return Post;
    } else if (name == "_FILES") {
      return Files;
    } else if (name == "_COOKIE") {
      return Cookie;
    } else if (name == "_SESSION") {
      return Session;
    } else if (name == "_REQUEST") {
      return Request;
    } else if (name == "_ENV") {
      return Env;
    }
  } else if (name == "http_response_header") {
    return HttpResponseHeader;
  }
  return Normal;
}

Block::Block() {}

Block::Block(const vector<StaticStatementPtr> &stat) {
  for (vector<StaticStatementPtr>::const_iterator it = stat.begin();
       it != stat.end(); ++it) {
    declareStaticStatement(*it);
  }
}

Block::~Block() {}

void Block::declareStaticStatement(StaticStatementPtr stat) {
  for (vector<StaticVariablePtr>::const_iterator it = stat->vars().begin();
       it != stat->vars().end(); ++it) {
    m_staticStmts[(*it)->name()->getStatic().data()] = (*it)->val();
  }
}

int Block::declareVariable(CStrRef var) {
  string svar(var.data(), var.size());
  VariableIndices::const_iterator it = m_variableIndices.find(svar);
  if (it == m_variableIndices.end()) {
    int i = m_variableIndices.size();
    m_variableIndices[svar].set(svar, i);
    return i;
  }
  return it->second.idx();
}

const Block::VariableIndices &Block::varIndices() const {
  return m_variableIndices;
}

// PHP is insane
Variant Block::getStaticValue(VariableEnvironment &env,
                              const char *name) const {
  map<string, ExpressionPtr>::const_iterator it = m_staticStmts.find(name);
  if (it != m_staticStmts.end() && it->second) {
    return it->second->eval(env);
  }
  return Variant();
}

///////////////////////////////////////////////////////////////////////////////
}
}
