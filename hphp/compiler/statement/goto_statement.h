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

#ifndef incl_HPHP_GOTO_STATEMENT_H_
#define incl_HPHP_GOTO_STATEMENT_H_

#include "hphp/parser/parser.h"
#include "hphp/compiler/statement/statement.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(GotoStatement);

class GotoStatement : public Statement {
public:
  GotoStatement(STATEMENT_CONSTRUCTOR_PARAMETERS, const std::string &label);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;

  const std::string &label() { return m_label; }

  void invalidate(ParserBase::GotoError error);
  int getId() const { return m_id; }
  void setId(int id) { m_id = id; }
  void setLabel(const std::string &label) { m_label = label; }
private:
  std::string m_label;
  ParserBase::GotoError m_error;
  int m_id;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_GOTO_STATEMENT_H_
