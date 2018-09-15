/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_SIMPLE_VARIABLE_H_
#define incl_HPHP_SIMPLE_VARIABLE_H_

#include "hphp/compiler/expression/expression.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(SimpleVariable);

struct SimpleVariable : Expression {
  SimpleVariable(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                 const std::string &name,
                 const std::string &docComment = "");

  DECLARE_BASE_EXPRESSION_VIRTUAL_FUNCTIONS;
  void analyzeProgram(AnalysisResultConstRawPtr ar) override;
  bool isThis() const override { return m_this;}
  bool isSuperGlobal() const { return m_superGlobal || m_globals; }
  bool isRefable(bool checkError = false) const override {
    return checkError || !m_this;
  }

  void setContext(Context context) override;

  const std::string &getName() const { return m_name;}
  const std::string &getDocComment() const {
    return m_docComment;
  }

  bool getAlwaysStash() const { return m_alwaysStash; }
  void setAlwaysStash() { m_alwaysStash = true; }
private:
  std::string m_name;
  std::string m_docComment;

  unsigned m_this : 1; // whether this is a legitimate $this
  unsigned m_globals : 1; // whether is is $GLOBAL
  unsigned m_superGlobal : 1;
  unsigned m_alwaysStash : 1;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_SIMPLE_VARIABLE_H_
