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

#ifndef incl_HPHP_MODIFIER_EXPRESSION_H_
#define incl_HPHP_MODIFIER_EXPRESSION_H_

#include "hphp/compiler/expression/expression.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ModifierExpression);

class ModifierExpression : public Expression {
public:
  explicit ModifierExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS);

  DECLARE_BASE_EXPRESSION_VIRTUAL_FUNCTIONS;

  void add(int modifier);
  int getCount() const { return m_modifiers.size();}
  int operator[](int index);

  /**
   * Whether the modifiers combine to mean public, including the implicit
   * public access that occurs when none of public|private|protected are
   * supplied.
   */
  bool isPublic() const;
  /**
   * Use isPublic unless you care about the difference between no-modifiers
   * implicit public and public-keyword-appears explicitly public.
   */
  bool isExplicitlyPublic() const;
  bool isProtected() const;
  bool isPrivate() const;
  bool isStatic() const;
  bool isAbstract() const;
  bool isFinal() const;
  bool isAsync() const;

  int getLocalEffects() const { return NoEffect; }

  bool validForFunction() const;
  bool validForClosure() const;
  bool validForTraitAliasRule() const;

  void setHasPrivacy(bool f) { m_hasPrivacy = f; }

private:
  std::vector<int> m_modifiers;
  bool m_hasPrivacy;

  bool hasModifier(int modifier) const;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_MODIFIER_EXPRESSION_H_
