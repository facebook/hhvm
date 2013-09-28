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

#include "hphp/compiler/expression/generation_label.h"

#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/expression/expression.h"

namespace HPHP {

GenerationLabel::GenerationLabel(Expression* e)
  : m_id(-1)
  , m_generation(-1)
  , m_owner(e)
{}

int GenerationLabel::id() const {
  assert(m_id >= 1);
  assert(m_generation ==
         m_owner->getFunctionScope()->getYieldLabelGeneration());
  return m_id;
}

void GenerationLabel::setNew() {
  auto func = m_owner->getFunctionScope();
  auto newGen = func->getYieldLabelGeneration();
  assert(m_generation < newGen);
  m_generation = newGen;
  m_id = func->allocYieldLabel();
}

void GenerationLabel::setExpression(Expression* e) {
  m_owner = e;
}

}
