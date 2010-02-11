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

#ifndef __EXPRESSION_LIST_H__
#define __EXPRESSION_LIST_H__

#include <lib/expression/expression.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ExpressionList);

class ExpressionList : public Expression {
public:
  ExpressionList(EXPRESSION_CONSTRUCTOR_PARAMETERS);

  // change case to lower so to make it case insensitive
  void toLower();

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;
  void analyzeProgramStart(AnalysisResultPtr ar);
  void analyzeProgramEnd(AnalysisResultPtr ar);

  virtual void addElement(ExpressionPtr exp);
  virtual void insertElement(ExpressionPtr exp, int index = 0);
  virtual bool isScalar() const;
  virtual bool hasEffect() const;
  bool isNoObjectInvolved() const;
  virtual bool containsDynamicConstant(AnalysisResultPtr ar) const;
  void removeElement(int index);
  virtual bool getScalarValue(Variant &value);

  bool isScalarArrayPairs() const;

  int getCount() const { return m_exps.size();}
  ExpressionPtr &operator[](int index);

  void getStrings(std::vector<std::string> &strings);

  void controlOrder(int withObj = 0);
  bool controllingOrder() const;
  int tempOffset() const {
    return m_tempStart +
      (m_controlOrder >= 2 ? 1 : 0);
  }

  /**
   * When a function call has too many arguments, we only want to output
   * max number of arguments, by limiting output count of subexpressions.
   */
  void setOutputCount(int count);
  int getOutputCount() const;
  void resetOutputCount();

  bool outputCPPTooManyArgsPre(CodeGenerator &cg, AnalysisResultPtr ar,
                               const std::string &name);
  void outputCPPTooManyArgsPost(CodeGenerator &cg, AnalysisResultPtr ar,
                                bool voidReturn);

  int outputCPPControlledEvalOrderPre(CodeGenerator &cg,
                                      AnalysisResultPtr ar);
  void outputCPPControlledEvalOrderPost(CodeGenerator &cg,
                                        AnalysisResultPtr ar);

private:
  ExpressionPtrVec m_exps;
  int m_outputCount;
  int m_controlOrder;
  int m_tempStart;
  bool m_arrayElements;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __EXPRESSION_LIST_H__
