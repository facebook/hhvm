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

#include <runtime/eval/ast/expression.h>
#include <runtime/eval/analysis/block.h>
#include <runtime/eval/ast/static_statement.h>
#include <runtime/eval/ast/name.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

void VariableIndex::set(CStrRef name, int idx) {
  m_idx = idx;
  m_sg = isSuperGlobal(name);
}


static String s_GLOBALS;
static String s__SERVER;
static String s__GET;
static String s__POST;
static String s__FILES;
static String s__COOKIE;
static String s__SESSION;
static String s__REQUEST;
static String s__ENV;
static String s_http_response_header;

static bool loaded = false;
void VariableIndex::SetupSuperGlobals() {
  s_GLOBALS = StringData::GetStaticString("GLOBALS");
  s__SERVER = StringData::GetStaticString("_SERVER");
  s__GET = StringData::GetStaticString("_GET");
  s__POST = StringData::GetStaticString("_POST");
  s__FILES = StringData::GetStaticString("_FILES");
  s__COOKIE = StringData::GetStaticString("_COOKIE");
  s__SESSION = StringData::GetStaticString("_SESSION");
  s__REQUEST = StringData::GetStaticString("_REQUEST");
  s__ENV = StringData::GetStaticString("_ENV");
  s_http_response_header = StringData::GetStaticString("http_response_header");
  loaded = true;
}

SuperGlobal VariableIndex::isSuperGlobal(CStrRef name) {
  ASSERT(loaded);
  if (name == s_GLOBALS) {
    return SgGlobals;
  } else if (name.data()[0] == '_') {
    if (name == s__SERVER) {
      return SgServer;
    } else if (name == s__GET) {
      return SgGet;
    } else if (name == s__POST) {
      return SgPost;
    } else if (name == s__FILES) {
      return SgFiles;
    } else if (name == s__COOKIE) {
      return SgCookie;
    } else if (name == s__SESSION) {
      return SgSession;
    } else if (name == s__REQUEST) {
      return SgRequest;
    } else if (name == s__ENV) {
      return SgEnv;
    }
  } else if (name == s_http_response_header) {
    return SgHttpResponseHeader;
  }
  return SgNormal;
}

Block::Block() {}

Block::Block(const vector<StaticStatementPtr> &stat,
             const Block::VariableIndices &variableIndices) {
  for (vector<StaticStatementPtr>::const_iterator it = stat.begin();
       it != stat.end(); ++it) {
    declareStaticStatement(*it);
  }
  m_variableIndices = variableIndices;
}

Block::~Block() {}

void Block::declareStaticStatement(StaticStatementPtr stat) {
  for (vector<StaticVariablePtr>::const_iterator it = stat->vars().begin();
       it != stat->vars().end(); ++it) {
    m_staticStmts[(*it)->name()->get()] = (*it)->val();
  }
}

int Block::declareVariable(CStrRef var) {
  ASSERT(var->isStatic());
  VariableIndices::const_iterator it = m_variableIndices.find(var);
  if (it == m_variableIndices.end()) {
    int i = m_variableIndices.size();
    m_variableIndices[var].set(var, i);
    m_variables.push_back(var.get());
    return i;
  }
  return it->second.idx();
}

const Block::VariableIndices &Block::varIndices() const {
  return m_variableIndices;
}

// PHP is insane
Variant Block::getStaticValue(VariableEnvironment &env, CStrRef name) const {
  StringMap<ExpressionPtr>::const_iterator it = m_staticStmts.find(name);
  if (it != m_staticStmts.end() && it->second) {
    return it->second->eval(env);
  }
  return Variant();
}

///////////////////////////////////////////////////////////////////////////////
}
}
