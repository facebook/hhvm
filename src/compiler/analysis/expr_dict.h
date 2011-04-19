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

#ifndef __EXPR_DICT_H__
#define __EXPR_DICT_H__

#include <compiler/analysis/dictionary.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ExprDict : public Dictionary {
public:
  ExprDict(AliasManager &am) : Dictionary(am) {}
  /* Building the dictionary */
  void build(MethodStatementPtr m);
  void visit(ExpressionPtr e);

  /* Computing the attributes */
  void beginBlock(ControlBlock *b);
  void endBlock(ControlBlock *b);
  void updateAccess(ExpressionPtr e);

  /* Copy propagation */
  void beforePropagate(ControlBlock *b);
  ExpressionPtr propagate(ExpressionPtr e);
private:
  std::vector<ExpressionRawPtr> m_avlExpr;
  std::vector<ExpressionRawPtr> m_avlAccess;
  ExpressionPtr m_active;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __EXPR_DICT_H__
