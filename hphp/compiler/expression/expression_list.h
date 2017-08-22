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

#ifndef incl_HPHP_EXPRESSION_LIST_H_
#define incl_HPHP_EXPRESSION_LIST_H_

#include "hphp/compiler/expression/array_pair_expression.h"
#include "hphp/compiler/expression/expression.h"
#include "hphp/runtime/base/type-variant.h"
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ExpressionList);

struct ExpressionList : Expression {
  enum ListKind {
    ListKindParam,
    ListKindComma,
    ListKindWrapped,
    ListKindWrappedNoWarn,
    ListKindLeft
  };

  using iterator = std::vector<ExpressionPtr>::iterator;

  explicit ExpressionList(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                          ListKind kind = ListKindParam);

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;
  ExpressionPtr preOptimize(AnalysisResultConstRawPtr ar) override;

  void setContext(Context context) override;
  void setListKind(ListKind kind) { m_kind = kind; }
  ListKind getListKind() const { return m_kind; }
  void addElement(ExpressionPtr exp) override;
  void insertElement(ExpressionPtr exp, int index = 0) override;
  bool isScalar() const override;
  int getLocalEffects() const override { return NoEffect; }
  bool isNoObjectInvolved() const;
  void removeElement(int index);
  void clearElements();
  bool getScalarValue(Variant &value) override;
  bool isRefable(bool checkError = false) const override;
  bool kidUnused(int i) const override;
  ExpressionPtr listValue() const;
  bool isLiteralString() const override;
  std::string getLiteralString() const override;

  bool isScalarArrayPairs() const;
  bool isSetCollectionScalar() const;

  int getCount() const { return m_exps.size();}
  ExpressionPtr &operator[](int index);
  iterator begin() { return m_exps.begin(); }
  iterator end() { return m_exps.end(); }

  void getStrings(std::vector<std::string> &strings);
  void stripConcat();

  void markParam(int p);
  void markParams();

  void setCollectionElems();
  void setContainsUnpack() { m_argUnpack = true; };
  bool containsUnpack() const { return m_argUnpack; }

  /**
   * Checks whether the expression list contains only literal strings and
   * (recursive) arrays of literal strings. Also returns the list of strings
   * if so.
   */
  bool flattenLiteralStrings(std::vector<ExpressionPtr> &literals) const;

  template <typename F> bool getListScalars(F) const;
private:
  void optimize(AnalysisResultConstRawPtr ar);
  unsigned int checkLitstrKeys() const;
  enum class ElemsKind: uint8_t { None, ArrayPairs, Collection };

private:
  std::vector<ExpressionPtr> m_exps;
  ElemsKind m_elems_kind;
  bool m_argUnpack;
  ListKind m_kind;
};

template <typename F>
inline bool ExpressionList::getListScalars(F f) const {
  if (m_elems_kind == ElemsKind::None) {
    for (const auto ape : m_exps) {
      Variant v;
      if (!ape->getScalarValue(v)) return false;
      if (!f(Variant{}, v)) return false;
    }
    return true;
  }
  if (!isScalarArrayPairs()) return false;
  for (const auto ape : m_exps) {
    auto exp = dynamic_pointer_cast<ArrayPairExpression>(ape);
    if (!exp) return false;
    auto name = exp->getName();
    auto val = exp->getValue();
    if (!name) {
      Variant v;
      auto const ret = val->getScalarValue(v);
      always_assert(ret);
      if (!f(Variant{}, v)) return false;
    } else {
      Variant n;
      Variant v;
      bool ret1 = name->getScalarValue(n);
      bool ret2 = val->getScalarValue(v);
      if (!(ret1 && ret2)) return false;
      if (!f(n, v)) return false;
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_EXPRESSION_LIST_H_
