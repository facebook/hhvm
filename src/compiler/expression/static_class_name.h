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

#ifndef __STATIC_CLASS_NAME_H__
#define __STATIC_CLASS_NAME_H__

#include <compiler/expression/expression.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class StaticClassName {
public:
  StaticClassName(ExpressionPtr classExp);

  bool isSelf() const { return m_isSelf; }
  bool isParent() const { return m_isParent; }
  bool isStatic() const { return m_isStatic; }
  bool isRedeclared() const { return m_redeclared; }
  bool isPresent() const { return m_present; }

  void setRedeclared() { m_redeclared = true; }

  ClassScopePtr resolveClass(BlockScopeRawPtr scope);
  bool checkPresent(BlockScopeRawPtr scope);
protected:
  ExpressionPtr m_class;
  std::string m_origClassName;
  std::string m_className;

  void updateClassName();
  void outputPHP(CodeGenerator &cg, AnalysisResultPtr ar);
private:
  unsigned m_isSelf : 1;
  unsigned m_isParent : 1;
  unsigned m_isStatic : 1;
  unsigned m_redeclared : 1;
  unsigned m_present : 1;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __STATIC_CLASS_NAME_H__
