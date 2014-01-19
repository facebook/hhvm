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

#ifndef incl_HPHP_LABEL_STATEMENT_H_
#define incl_HPHP_LABEL_STATEMENT_H_

#include "hphp/compiler/statement/statement.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(LabelStatement);

class LabelStatement : public Statement {
public:
  LabelStatement(STATEMENT_CONSTRUCTOR_PARAMETERS, const std::string &label);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;

  const std::string &label() { return m_label; }

  void invalidate() { m_isValid = false; }
private:
  std::string m_label;
  bool m_isValid;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_LABEL_STATEMENT_H_
