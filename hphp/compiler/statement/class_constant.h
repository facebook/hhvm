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

#ifndef incl_HPHP_CLASS_CONSTANT_H_
#define incl_HPHP_CLASS_CONSTANT_H_

#include "hphp/compiler/type_annotation.h"
#include "hphp/compiler/statement/statement.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Array;

DECLARE_BOOST_TYPES(ExpressionList);
DECLARE_BOOST_TYPES(ClassConstant);

struct ClassConstant : Statement, IParseHandler {
  ClassConstant(STATEMENT_CONSTRUCTOR_PARAMETERS, std::string typeConstraint,
                ExpressionListPtr exp, bool abstract,
                bool typeconst, TypeAnnotationPtr typeAnnot);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;

  // implementing IParseHandler
  void onParseRecur(AnalysisResultConstRawPtr ar, FileScopeRawPtr fs,
                    ClassScopePtr scope) override;

  std::string getTypeConstraint() const { return m_typeConstraint; }
  Array getTypeStructure() const { return m_typeStructure; }

  ExpressionListPtr getConList() { return m_exp; }
  bool isAbstract() { return m_abstract; }
  bool isTypeconst() { return m_typeconst; }
private:
  std::string m_typeConstraint;
  ExpressionListPtr m_exp;
  bool m_abstract;
  bool m_typeconst;
  Array m_typeStructure;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_CLASS_CONSTANT_H_
