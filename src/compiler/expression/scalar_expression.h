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

#ifndef __SCALAR_EXPRESSION_H__
#define __SCALAR_EXPRESSION_H__

#include <compiler/expression/expression.h>
#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ScalarExpression);

class ScalarExpression : public Expression {
public:
  ScalarExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                   int type, const std::string &value, bool quoted = false);
  ScalarExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                   CVarRef value, bool quoted = true);

  // change case to lower so to make it case insensitive
  void toLower(bool funcCall = false);

  DECLARE_BASE_EXPRESSION_VIRTUAL_FUNCTIONS;
  ExpressionPtr postOptimize(AnalysisResultConstPtr ar);
  virtual int getLocalEffects() const { return NoEffect; }
  virtual bool isScalar() const { return true;}
  virtual bool isLiteralString() const;
  virtual std::string getLiteralString() const;
  TypePtr inferenceImpl(AnalysisResultConstPtr ar, TypePtr type,
                        bool coerce);
  virtual TypePtr inferAndCheck(AnalysisResultPtr ar, TypePtr type,
                                bool coerce);
  virtual bool getScalarValue(Variant &value) {
    value = getVariant(); return true;
  }
  virtual unsigned getCanonHash() const;
  virtual bool canonCompare(ExpressionPtr e) const;
  bool isQuoted() const { return m_quoted; }

  int getType() const { return m_type;}
  const std::string &getString() const { return m_value;}
  void appendEncapString(const std::string &value);
  bool isLiteralInteger() const;

  int64 getLiteralInteger() const;
  std::string getIdentifier() const;
  Variant getVariant();
  int64 getHash() const;

  void setComment(const std::string &comment) { m_comment = comment;}
  const std::string getComment() { return m_comment;}

  void outputCPPString(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputCPPInteger(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputCPPDouble(CodeGenerator &cg, AnalysisResultPtr ar);

  std::string getCPPLiteralString(CodeGenerator &cg, bool *binary = NULL);

  bool getString(const std::string *&s) const;
  bool getInt(int64 &i) const;
  bool getDouble(double &d) const;
private:
  int m_type;
  std::string m_serializedValue;
  double m_dval;
  std::string m_value;
  std::string m_originalValue;
  std::string m_translated;
  bool m_quoted;
  std::string m_comment; // for inlined constant name
  void outputCPPString(const std::string &str, CodeGenerator &cg,
                       AnalysisResultPtr ar, bool constant);
  void outputCPPNamedInteger(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputCPPNamedDouble(CodeGenerator &cg, AnalysisResultPtr ar);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __SCALAR_EXPRESSION_H__
