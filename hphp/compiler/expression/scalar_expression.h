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

#ifndef incl_HPHP_SCALAR_EXPRESSION_H_
#define incl_HPHP_SCALAR_EXPRESSION_H_

#include "hphp/compiler/expression/expression.h"
#include "hphp/runtime/base/complex-types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ScalarExpression);

class ScalarExpression : public Expression {
public:
  ScalarExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                   int type, const std::string &value, bool quoted = false);
  ScalarExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                   int type, const std::string &value,
                   const std::string &translated, bool quoted = false);
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
  std::string getOriginalLiteralString() const;
  std::string getLiteralStringImpl(bool original) const;
  bool needsTranslation() const;
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
  const std::string &getOriginalString() const { return m_originalValue; }
  void appendEncapString(const std::string &value);
  bool isLiteralInteger() const;

  int64_t getLiteralInteger() const;
  std::string getIdentifier() const;
  Variant getVariant() const;
  int64_t getHash() const;

  void setComment(const std::string &comment) { m_comment = comment;}
  std::string getComment() { return m_comment;}


  bool getString(const std::string *&s) const;
  bool getInt(int64_t &i) const;
  bool getDouble(double &d) const;

  void setCompilerHaltOffset(int64_t ofs);
private:
  int m_type;
  std::string m_serializedValue;
  double m_dval;
  std::string m_value;
  std::string m_originalValue;
  std::string m_translated;
  bool m_quoted;
  std::string m_comment; // for inlined constant name
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_SCALAR_EXPRESSION_H_
