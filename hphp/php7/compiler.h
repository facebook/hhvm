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

#ifndef incl_HPHP_PHP7_COMPILER_H_
#define incl_HPHP_PHP7_COMPILER_H_

#include <hphp/php7/zend/zend.h>
#include <hphp/php7/ast_info.h>
#include <hphp/php7/unit.h>

#include <string>

namespace HPHP { namespace PHP7 {

struct Compiler {
  explicit Compiler();

  static std::unique_ptr<Unit> compile(zend_ast* ast) {
    Compiler compiler;
    compiler.compileProgram(ast);
    return std::move(compiler.unit);
  }

 private:
  void compileProgram(zend_ast* ast);
  void compileStatement(zend_ast* ast);
  void compileExpression(zend_ast* ast);
  void compileZvalLiteral(zval* ast);

  [[noreturn]]
  void panic(const std::string& msg);

  std::unique_ptr<Unit> unit;

  Block* activeBlock;
};

}} // HPHP::PHP7


#endif // incl_HPHP_PHP7_COMPILER_H_
