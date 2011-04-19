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

#ifndef __LOOP_STATEMENT_H__
#define __LOOP_STATEMENT_H__

#include <compiler/statement/statement.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(LoopStatement);

class LoopStatement : public Statement {
 public:
  LoopStatement(STATEMENT_CONSTRUCTOR_PARAMETERS);
  void clearStringBufs();
  void addStringBuf(const std::string &name);
  void removeStringBuf(const std::string &name);
  int numStringBufs() const { return m_string_bufs.size(); }
  void cppDeclareBufs(CodeGenerator &cg, AnalysisResultPtr ar);
  void cppEndBufs(CodeGenerator &cg, AnalysisResultPtr ar);
  bool checkStringBuf(const std::string &name);
 private:
  std::set<std::string>  m_string_bufs;
  LoopStatementWeakPtr m_outer;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __LOOP_STATEMENT_H__
