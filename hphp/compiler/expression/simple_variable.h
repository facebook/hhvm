/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

class Symbol;
DECLARE_BOOST_TYPES(SimpleVariable);

class SimpleVariable : public Expression {
public:
  SimpleVariable(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                 const std::string &name,
                 const std::string &docComment = "");

  DECLARE_BASE_EXPRESSION_VIRTUAL_FUNCTIONS;
  virtual int getLocalEffects() const;
  virtual bool isThis() const { return m_this;}
  bool isSuperGlobal() const { return m_superGlobal || m_globals; }
  virtual bool isRefable(bool checkError = false) const {
    return checkError || !m_this;
  }
  virtual TypePtr inferAndCheck(AnalysisResultPtr ar, TypePtr type,
                                bool coerce);

  virtual bool canonCompare(ExpressionPtr e) const;
  virtual void setContext(Context context);

  const std::string &getName() const { return m_name;}
  const std::string &getDocComment() const {
    return m_docComment;
  }
  Symbol *getSymbol() const { return m_sym; }

  bool couldBeAliased() const;
  bool canKill(bool unset) const;
  bool isHidden() const;
  bool checkUnused() const;
  bool getAlwaysStash() const { return m_alwaysStash; }
  void setAlwaysStash() { m_alwaysStash = true; }
  void updateSymbol(SimpleVariablePtr src);
  void coalesce(SimpleVariablePtr other);
private:
  std::string m_name;
  std::string m_docComment;

  TypePtr m_superGlobalType;
  Symbol *m_sym;
  Symbol *m_originalSym;

  unsigned m_this : 1; // whether this is a legitimate $this
  unsigned m_globals : 1; // whether is is $GLOBAL
  unsigned m_superGlobal : 1;
  unsigned m_alwaysStash : 1;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_SIMPLE_VARIABLE_H_
