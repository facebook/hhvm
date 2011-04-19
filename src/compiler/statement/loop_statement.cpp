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

#include <compiler/statement/loop_statement.h>
#include <compiler/analysis/variable_table.h>

using namespace HPHP;

LoopStatement::LoopStatement(STATEMENT_CONSTRUCTOR_PARAMETERS) :
    Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES) {
}

void LoopStatement::addStringBuf(const std::string &name) {
  m_string_bufs.insert(name);
}

void LoopStatement::removeStringBuf(const std::string &name) {
  m_string_bufs.erase(name);
}

void LoopStatement::clearStringBufs() {
  m_string_bufs.clear();
}

void LoopStatement::cppDeclareBufs(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (numStringBufs()) {
    cg_indentBegin("{\n");
    for (std::set<std::string>::iterator it = m_string_bufs.begin(),
           end = m_string_bufs.end(); it != end; ++it) {
      const char *prefix =
        getScope()->getVariables()->getVariablePrefix(*it);
      cg_printf("StringBuffer %s_sbuf_%s%s(512);\n",
                Option::TempPrefix, prefix, it->c_str());
    }
    m_outer = cg.getLoopStatement();
    cg.setLoopStatement(boost::static_pointer_cast<LoopStatement>
                        (shared_from_this()));
  }
}

void LoopStatement::cppEndBufs(CodeGenerator &cg, AnalysisResultPtr ar) {
  if (numStringBufs()) {
    cg.setLoopStatement(m_outer.lock());
    m_outer.reset();
    for (std::set<std::string>::iterator it = m_string_bufs.begin(),
           end = m_string_bufs.end(); it != end; ++it) {

      const char *prefix =
        getScope()->getVariables()->getVariablePrefix(*it);

      cg_printf("concat_assign(%s%s, %s_sbuf_%s%s.detach());\n",
                prefix, it->c_str(),
                Option::Option::TempPrefix,
                prefix, it->c_str());
    }
    cg_indentEnd("}\n");
  }
}

bool LoopStatement::checkStringBuf(const std::string &name) {
  if (m_string_bufs.find(name) != m_string_bufs.end()) {
    return true;
  }
  if (LoopStatementPtr outer = m_outer.lock()) {
    return outer->checkStringBuf(name);
  }
  return false;
}
