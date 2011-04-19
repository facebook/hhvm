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

#ifndef __EXPRESSION_LIST_H__
#define __EXPRESSION_LIST_H__

#include <compiler/expression/expression.h>

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

  ExpressionList(EXPRESSION_CONSTRUCTOR_PARAMETERS,
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

  bool isScalarArrayPairs() const;

  int getCount() const { return m_exps.size();}
  ExpressionPtr &operator[](int index);

  void getStrings(std::vector<std::string> &strings);
  void stripConcat();

  void markParam(int p, bool noRefWrapper);
  void markParams(bool noRefWrapper);

  /**
   * When a function call has too many arguments, we only want to output
   * max number of arguments, by limiting output count of subexpressions.
   */
  void setOutputCount(int count);
  int getOutputCount() const;
  void resetOutputCount();

  virtual bool canonCompare(ExpressionPtr e) const;

  void preOutputStash(CodeGenerator &cg, AnalysisResultPtr ar,
                      int state);
  bool preOutputCPP(CodeGenerator &cg, AnalysisResultPtr ar,
                    int state);
  bool outputCPPUnneeded(CodeGenerator &cg, AnalysisResultPtr ar);
  bool hasNonArrayCreateValue(bool arrayElements = true,
                              unsigned int start = 0) const;
  void outputCPPUniqLitKeyArrayInit(CodeGenerator &cg,
                                    AnalysisResultPtr ar,
                                    bool litstrKeys,
                                    int64 max,
                                    bool arrayElements = true,
                                    unsigned int start = 0);
  /**
   * Checks whether the expression list contains only literal strings and
   * (recursive) arrays of literal strings. Also returns the list of strings
   * if so.
   */
  bool flattenLiteralStrings(std::vector<ExpressionPtr> &literals) const;

private:
  void optimize(AnalysisResultConstPtr ar);
  unsigned int checkLitstrKeys() const;
  bool outputCPPArrayCreate(CodeGenerator &cg, AnalysisResultPtr ar,
                            bool isVector, bool pre);
  bool outputCPPInternal(CodeGenerator &cg,
                         AnalysisResultPtr ar, bool needed, bool pre);

  ExpressionPtrVec m_exps;
  int m_outputCount;
  bool m_arrayElements;
  ListKind m_kind;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __EXPRESSION_LIST_H__
