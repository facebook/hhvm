/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_BREAK_STATEMENT_H_
#define incl_HPHP_BREAK_STATEMENT_H_

#include "hphp/compiler/statement/statement.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(BreakStatement);

struct BreakStatement : Statement {
protected:
  BreakStatement(STATEMENT_CONSTRUCTOR_BASE_PARAMETERS, uint64_t depth);
public:
  BreakStatement(STATEMENT_CONSTRUCTOR_PARAMETERS, uint64_t depth);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;
  StatementPtr preOptimize(AnalysisResultConstPtr ar) override;
  uint64_t getDepth();
protected:
  const char *m_name;
  uint64_t m_depth;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_BREAK_STATEMENT_H_
