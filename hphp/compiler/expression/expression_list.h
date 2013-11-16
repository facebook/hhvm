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

#ifndef incl_HPHP_EXPRESSION_LIST_H_
#define incl_HPHP_EXPRESSION_LIST_H_

#include "hphp/compiler/expression/expression.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ExpressionList);

class ExpressionList : public Expression {
public:
  enum ListKind {
    ListKindParam,
    ListKindComma,
    ListKindWrapped,
    ListKindLeft
  };

  explicit ExpressionList(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                          ListKind kind = ListKindParam);

  // change case to lower so to make it case insensitive
  void toLower();

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;
  ExpressionPtr preOptimize(AnalysisResultConstPtr ar);
  ExpressionPtr postOptimize(AnalysisResultConstPtr ar);

  virtual void setContext(Context context);
  void setListKind(ListKind kind) { m_kind = kind; }
  ListKind getListKind() const { return m_kind; }
  virtual void addElement(ExpressionPtr exp);
  virtual void insertElement(ExpressionPtr exp, int index = 0);
  virtual bool isScalar() const;
  virtual int getLocalEffects() const { return NoEffect; }
  bool isNoObjectInvolved() const;
  virtual bool containsDynamicConstant(AnalysisResultPtr ar) const;
  void removeElement(int index);
  void clearElements();
  virtual bool getScalarValue(Variant &value);
  virtual bool isRefable(bool checkError = false) const;
  virtual bool kidUnused(int i) const;
  ExpressionPtr listValue() const;
  virtual bool isLiteralString() const;
  virtual std::string getLiteralString() const;

  bool isScalarArrayPairs() const;

  int getCount() const { return m_exps.size();}
  ExpressionPtr &operator[](int index);

  void getStrings(std::vector<std::string> &strings);
  void getOriginalStrings(std::vector<std::string> &strings);
  void stripConcat();

  void markParam(int p, bool noRefWrapper);
  void markParams(bool noRefWrapper);

  void setCollectionType(int cType);

  virtual bool canonCompare(ExpressionPtr e) const;

  /**
   * Checks whether the expression list contains only literal strings and
   * (recursive) arrays of literal strings. Also returns the list of strings
   * if so.
   */
  bool flattenLiteralStrings(std::vector<ExpressionPtr> &literals) const;

private:
  void optimize(AnalysisResultConstPtr ar);
  unsigned int checkLitstrKeys() const;

  ExpressionPtrVec m_exps;
  bool m_arrayElements;
  int m_collectionType;
  ListKind m_kind;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_EXPRESSION_LIST_H_
