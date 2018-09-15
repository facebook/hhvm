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

#ifndef incl_HPHP_COMPILER_TYPEDEF_STATEMENT_H_
#define incl_HPHP_COMPILER_TYPEDEF_STATEMENT_H_

#include <string>
#include <memory>

#include "hphp/compiler/statement/statement.h"
#include "hphp/compiler/expression/expression.h"
#include "hphp/compiler/type_annotation.h"

namespace HPHP {
//////////////////////////////////////////////////////////////////////
DECLARE_BOOST_TYPES(ExpressionList);

struct TypedefStatement : Statement, IParseHandler {
  explicit TypedefStatement(STATEMENT_CONSTRUCTOR_PARAMETERS,
                            const std::string& name,
                            const ExpressionListPtr& attrList,
                            const TypeAnnotationPtr& typeAnnotation);
  ~TypedefStatement() override;

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;

public: // IParseHandler
  void onParse(AnalysisResultConstRawPtr, FileScopePtr) override;

public:
  const std::string name;
  const ExpressionListPtr attrList;
  const TypeAnnotationPtr annot;
};

typedef std::shared_ptr<TypedefStatement> TypedefStatementPtr;

//////////////////////////////////////////////////////////////////////
}

#endif
