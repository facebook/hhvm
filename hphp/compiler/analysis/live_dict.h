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

#ifndef incl_HPHP_LIVE_DICT_H_
#define incl_HPHP_LIVE_DICT_H_

#include "hphp/compiler/analysis/dictionary.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

//DECLARE_BOOST_TYPES(Type);

class LiveDict : public Dictionary {
public:
  explicit LiveDict(AliasManager &am) : Dictionary(am) {}
  /* Building the dictionary */
  void build(MethodStatementPtr m);
  void visit(ExpressionPtr e);

  /* Computing the attributes */
  void beginBlock(ControlBlock *b);
  void endBlock(ControlBlock *b);
  void updateAccess(ExpressionPtr e);
  void updateParams();
  void buildConflicts();
  bool color(TypePtr type);
  void coalesce(MethodStatementPtr m);
  bool shrinkWrap();
private:
  void addConflicts(size_t width, BitOps::Bits *live, BitOps::Bits *dying);
  ExpressionPtr m_refs;

  BitOps::Bits  *m_dying;
  bool          m_getVars;
  bool          m_coalesce;
  BitSetVec     m_conflicts;
  std::map<int,int> m_remap;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_LIVE_DICT_H_
