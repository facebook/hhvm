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

#ifndef incl_HPHP_CONSTANT_EXPRESSION_H_
#define incl_HPHP_CONSTANT_EXPRESSION_H_

#include "hphp/compiler/expression/expression.h"

#define CONSTANT(value) makeConstant(ar, value)

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ConstantExpression);

class ConstantExpression : public Expression, IParseHandler {
public:
  ConstantExpression(EXPRESSION_CONSTRUCTOR_PARAMETERS,
                     const std::string &name,
                     bool hadBackslash,
                     const std::string &docComment = "");

  DECLARE_BASE_EXPRESSION_VIRTUAL_FUNCTIONS;
  void onParse(AnalysisResultConstPtr ar, FileScopePtr scope);
  ExpressionPtr preOptimize(AnalysisResultConstPtr ar);
  virtual bool isTemporary() const {
    return isNull() || isBoolean();
  }
  virtual bool isScalar() const;
  virtual bool isLiteralNull() const;
  virtual int getLocalEffects() const { return NoEffect; }
  virtual bool getScalarValue(Variant &value);
  virtual bool containsDynamicConstant(AnalysisResultPtr ar) const {
    return !m_valid || m_dynamic;
  }

  virtual unsigned getCanonHash() const;
  virtual bool canonCompare(ExpressionPtr e) const;

  const std::string &getName() const { return m_name;}
  const std::string &getOriginalName() const { return m_origName;}
  const std::string getNonNSOriginalName() const {
    auto nsPos = m_origName.rfind('\\');
    if (nsPos == string::npos) {
      return m_origName;
    }
    return m_origName.substr(nsPos + 1);
  }
  const std::string &getDocComment() const {
    return m_docComment;
  }

  bool isNull() const;
  bool isBoolean() const;
  bool isDouble() const;
  bool getBooleanValue() const;
  void pushConst(const std::string &name);
  void popConst();
  void setComment(const std::string &comment) { m_comment = comment;}
  std::string getComment() { return m_comment;}
  bool isValid() const { return m_valid; }
  bool isDynamic() const { return m_dynamic; }
  bool hadBackslash() const { return m_hadBackslash; }
private:

  Symbol *resolveNS(AnalysisResultConstPtr ar);
  std::string m_name;
  std::string m_origName;
  bool m_hadBackslash;
  std::string m_docComment;
  std::string m_comment; // for inlined constant name
  bool m_valid;
  bool m_dynamic;
  bool m_visited;
  bool m_depsSet;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_CONSTANT_EXPRESSION_H_
