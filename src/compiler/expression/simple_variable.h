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

#ifndef __SIMPLE_VARIABLE_H__
#define __SIMPLE_VARIABLE_H__

#include <compiler/expression/expression.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(SimpleVariable);

class SimpleVariable : public Expression {
public:
  SimpleVariable(EXPRESSION_CONSTRUCTOR_PARAMETERS, const std::string &name);

  DECLARE_BASE_EXPRESSION_VIRTUAL_FUNCTIONS;
  virtual int getLocalEffects() const { return NoEffect; }
  virtual bool isThis() const { return m_this;}
  virtual bool isRefable(bool checkError = false) const { return true;}
  virtual TypePtr inferAndCheck(AnalysisResultPtr ar, TypePtr type,
                                bool coerce);

  virtual bool canonCompare(ExpressionPtr e) const;
  const std::string &getName() const { return m_name;}
  void preOutputStash(CodeGenerator &cg, AnalysisResultPtr ar,
                      int state);
private:
  std::string m_name;
  std::string m_text;
  bool m_this; // whether this is a legitimate $this
  bool m_globals; // whether is is $GLOBAL
  bool m_superGlobal;
  TypePtr m_superGlobalType;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __SIMPLE_VARIABLE_H__
