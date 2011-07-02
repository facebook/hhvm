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

typedef std::pair<TypePtr, int>     TypePtrIdxPair;
typedef std::vector<TypePtrIdxPair> TypePtrIdxPairVec;

class ExprDict : public Dictionary {
public:
  ExprDict(AliasManager &am);
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
  TypePtr propagateType(ExpressionPtr e);

  void getTypes(ExpressionPtr e, TypePtrIdxPairVec &types);
private:

  /**
   * types is filled with (type assertion, canon id for that type assertion)
   * tuples
   */
  void getTypes(int id, TypePtrIdxPairVec &types);

  /**
   * entries is filled with (type assertion, canon id for that type assertion)
   * tuples or (TypePtr(), canon id) tuples
   */
  void getAllEntries(int id, TypePtrIdxPairVec &types);

  template <class T>
  void getEntries(int id, TypePtrIdxPairVec &entries, T func);

  bool containsAssertion(TypePtr assertion,
                         const TypePtrIdxPairVec &types,
                         TypePtrIdxPair &entry) const;

  inline bool isCanonicalStructure(int i) const { return id(i) == i; }

  int id(int i) const {
    ASSERT(i >= 0 && i < (int) m_canonIdMap.size());
    int ret = m_canonIdMap[i];
    ASSERT(ret != -1);
    return ret;
  }

  void setStructureOps(int idx, BitOps::Bits *bits, bool flag);

  std::vector<ExpressionRawPtr> m_avlExpr;
  std::vector<ExpressionRawPtr> m_avlAccess;
  std::vector<TypePtr>          m_avlTypes;
  std::vector<int>              m_avlTypeAsserts;

  TypePtr extractTypeAssertion(ExpressionPtr e) const;
  TypePtr reduceToSingleAssertion(const TypePtrIdxPairVec &types) const;

  TypePtrIdxPairVec m_canonTypeMap;
    // ids to (assertion type, prev id)

  std::vector<int> m_canonIdMap;
    // ids -> canon structure id

  ExpressionPtr m_active;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __EXPR_DICT_H__
