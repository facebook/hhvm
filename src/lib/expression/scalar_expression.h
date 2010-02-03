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

#ifndef __SCALAR_EXPRESSION_H__
#define __SCALAR_EXPRESSION_H__

#include <lib/expression/expression.h>
#include <cpp/base/type_variant.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ScalarExpression);

class ScalarExpression : public Expression, public IParseHandler {
  friend class ScalarExpressionHook;
public:
  ScalarExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                   int type, const std::string &value, bool quoted = false);
  ScalarExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                   CVarRef value, bool quoted = true);

  // change case to lower so to make it case insensitive
  void toLower();

  DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS;
  virtual bool hasEffect() const { return false;}
  virtual bool isScalar() const { return true;}
  virtual bool isLiteralString() const;
  virtual std::string getLiteralString() const;
  virtual TypePtr inferAndCheck(AnalysisResultPtr ar, TypePtr type,
                                bool coerce);
  virtual bool getScalarValue(Variant &value) {
    value = getVariant(); return true;
  }

  // implementing IParseHandler
  virtual void onParse(AnalysisResultPtr ar);

  int getType() const { return m_type;}
  const std::string &getString() const { return m_value;}
  void appendEncapString(const std::string &value);
  bool isLiteralInteger() const;

  int64 getLiteralInteger() const;
  std::string getIdentifier() const;
  Variant &getVariant();
  int64 getHash() const;

  void setComment(const std::string comment) { m_comment = comment;}
  const std::string getComment() { return m_comment;}

  void outputCPPString(CodeGenerator &cg, AnalysisResultPtr ar);

  static void setHookHandler(void (*hookHandler)(AnalysisResultPtr ar,
                                                 ScalarExpressionPtr sc,
                                                 HphpHookUniqueId id)) {
    m_hookHandler = hookHandler;
  }

private:
  int m_type;
  std::string m_value;
  std::string m_translated;
  bool m_quoted;
  Variant m_variant; // value created for compile time optimization
  std::string m_comment; // for inlined constant name

  // hook
  static void (*m_hookHandler)(AnalysisResultPtr ar,
                               ScalarExpressionPtr sc,
                               HphpHookUniqueId id);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __SCALAR_EXPRESSION_H__
