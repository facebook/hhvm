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

#ifndef __MODIFIER_EXPRESSION_H__
#define __MODIFIER_EXPRESSION_H__

#include <compiler/expression/expression.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ModifierExpression);

class ModifierExpression : public Expression {
public:
  ModifierExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS);

  DECLARE_BASE_EXPRESSION_VIRTUAL_FUNCTIONS;

  void add(int modifier);
  int getCount() const { return m_modifiers.size();}
  int operator[](int index);

  bool isPublic() const;
  bool isProtected() const;
  bool isPrivate() const;
  bool isStatic() const;
  bool isAbstract() const;
  bool isFinal() const;

private:
  std::vector<int> m_modifiers;

  bool hasModifier(int modifier) const;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __MODIFIER_EXPRESSION_H__
