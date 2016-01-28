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

#ifndef incl_HPHP_DECLARE_STATEMENT_H_
#define incl_HPHP_DECLARE_STATEMENT_H_

#include "hphp/compiler/statement/statement.h"
#include "hphp/compiler/statement/block_statement.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(DeclareStatement);

struct DeclareStatement : Statement {
  DeclareStatement(STATEMENT_CONSTRUCTOR_PARAMETERS);

  DECLARE_STATEMENT_VIRTUAL_FUNCTIONS;

  using DeclareMap = std::unordered_map<std::string, ExpressionPtr>;
  const DeclareMap& getDeclareMap() const { return m_declareMap; }
  BlockStatementPtr getBlock() const { return m_block; }

  void addDeclare(std::string str, ExpressionPtr expr) {
    m_declareMap.emplace(str, std::move(expr));
  }
  void setBlock(BlockStatementPtr block) { m_block = std::move(block); }

private:
  BlockStatementPtr m_block {nullptr};
  DeclareMap m_declareMap;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_DO_STATEMENT_H_
