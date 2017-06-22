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

#include <hphp/php7/compiler.h>

#include <iostream>

namespace HPHP { namespace PHP7 {

using namespace BC;

Compiler::Compiler() {
  unit = std::make_unique<Unit>();
  unit->name = "unit.hhas";
}

void Compiler::compileProgram(zend_ast* ast) {
  assert(ast->kind == ZEND_AST_STMT_LIST);
  auto list = zend_ast_get_list(ast);

  auto pseudomain = unit->getPseudomain();
  activeBlock = pseudomain->entry;

  for (int i = 0; i < list->children; i++) {
    compileStatement(list->child[i]);
  }

  if (activeBlock) {
    activeBlock->emit(Int{-1});
    activeBlock->emit(RetC{});
  }
}

void Compiler::panic(const std::string& msg) {
  std::cerr << "panic: " << msg <<std::endl;
  std::abort();
}

void Compiler::compileZvalLiteral(zval* zv) {
  switch (Z_TYPE_P(zv)) {
    case IS_LONG:
      activeBlock->emit(Int{Z_LVAL_P(zv)});
      break;
    default:
      panic("unsupported literal");
  }

}

void Compiler::compileExpression(zend_ast* ast) {
  switch (ast->kind) {
    case ZEND_AST_ZVAL:
      compileZvalLiteral(zend_ast_get_zval(ast));
      break;
    case ZEND_AST_BINARY_OP:
      compileExpression(ast->child[0]);
      compileExpression(ast->child[1]);
      switch (ast->attr) {
        case ZEND_ADD:
          activeBlock->emit(Add{});
          break;
        default:
          panic("unsupported binop");
      }
      break;
    default:
      panic("unsupported expression");
  }
}

void Compiler::compileStatement(zend_ast* ast) {
  switch (ast->kind) {
    case ZEND_AST_STMT_LIST: {
      // just a block, so recur please :)
      auto list = zend_ast_get_list(ast);
      for (int i = 0; i < list->children; i++) {
        compileStatement(list->child[i]);
      }
      break;
    }
    case ZEND_AST_ECHO:
      compileExpression(ast->child[0]);
      activeBlock->emit(Print{});
      activeBlock->emit(PopC{});
      break;
    default:
      panic("unsupported statement");
  }
}

}} // HPHP::PHP7
