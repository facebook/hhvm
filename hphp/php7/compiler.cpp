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

#include "hphp/php7/compiler.h"

#include <folly/Format.h>

#include <iostream>

namespace HPHP { namespace php7 {

namespace {
// helpers to cope with these functions taking non-const pointers
// in zend_ast.h

inline const zval* zend_ast_get_zval(const zend_ast* ast) {
  return &reinterpret_cast<const zend_ast_zval*>(ast)->val;
}

inline const zend_ast_list* zend_ast_get_list(const zend_ast* ast) {
  return reinterpret_cast<const zend_ast_list*>(ast);
}

} // namespace


using namespace bc;

Compiler::Compiler() {
  unit = std::make_unique<Unit>();
  unit->name = "unit.hhas";
}

void Compiler::compileProgram(const zend_ast* ast) {
  assert(ast->kind == ZEND_AST_STMT_LIST);
  auto list = zend_ast_get_list(ast);

  auto pseudomain = unit->getPseudomain();
  activeFunction = pseudomain;
  activeBlock = pseudomain->entry;

  for (uint32_t i = 0; i < list->children; i++) {
    compileStatement(list->child[i]);
  }

  if (activeBlock) {
    activeBlock->emit(Int{-1});
    activeBlock->exit(RetC{});
  }
}

void Compiler::panic(const std::string& msg) {
  throw CompilerException(folly::sformat("panic: {}", msg));
}

void Compiler::compileZvalLiteral(const zval* zv) {
  switch (Z_TYPE_P(zv)) {
    case IS_LONG:
      activeBlock->emit(Int{Z_LVAL_P(zv)});
      break;
    case IS_NULL:
      activeBlock->emit(Null{});
      break;
    case IS_FALSE:
      activeBlock->emit(False{});
      break;
    case IS_TRUE:
      activeBlock->emit(True{});
      break;
    case IS_DOUBLE:
      activeBlock->emit(Double{Z_DVAL_P(zv)});
      break;
    case IS_STRING:
      activeBlock->emit(String{Z_STRVAL_P(zv)});
      break;
    default:
      panic("unsupported literal");
  }
}

void Compiler::compileConstant(const zend_ast* ast) {
  auto name = ast->child[0];
  const char* str = Z_STRVAL_P(zend_ast_get_zval(name));
  if (name->attr & ZEND_NAME_NOT_FQ) {
    if (strcasecmp(str, "true") == 0) {
      activeBlock->emit(True{});
    } else if (strcasecmp(str, "false") == 0) {
      activeBlock->emit(False{});
    } else if (strcasecmp(str, "null") == 0) {
      activeBlock->emit(Null{});
    } else {
      panic("unknown unqualified constant");
    }
  } else {
    panic("unknown constant");
  }
}

Bytecode Compiler::opForBinaryOp(const zend_ast* ast) {
  // NB: bizarrely, greater-than (>,>=) have their own AST type
  // and there is no ZEND_IS_GREATER since it doesn't correspond to a VM
  // instruction
  if (ast->kind == ZEND_AST_GREATER) {
    return Gt{};
  } else if (ast->kind == ZEND_AST_GREATER_EQUAL) {
    return Gte{};
  }

  switch (ast->attr) {
    case ZEND_ADD: return Add{};
    case ZEND_SUB: return Sub{};
    case ZEND_MUL: return Mul{};
    case ZEND_DIV: return Div{};
    case ZEND_POW: return Pow{};
    case ZEND_MOD: return Mod{};
    case ZEND_SL: return Shl{};
    case ZEND_SR: return Shr{};
    case ZEND_BW_OR: return BitOr{};
    case ZEND_BW_AND: return BitAnd{};
    case ZEND_BW_XOR: return BitXor{};
    case ZEND_CONCAT: return Concat{};

    case ZEND_IS_IDENTICAL: return Same{};
    case ZEND_IS_NOT_IDENTICAL: return NSame{};
    case ZEND_IS_EQUAL: return Eq{};
    case ZEND_IS_NOT_EQUAL: return Neq{};
    case ZEND_IS_SMALLER: return Lt{};
    case ZEND_IS_SMALLER_OR_EQUAL: return Lte{};
    case ZEND_SPACESHIP: return Cmp{};
    default:
      panic("unknown binop");
  }
}

void Compiler::compileUnaryOp(const zend_ast* ast) {
  switch (ast->kind) {
    case ZEND_AST_UNARY_MINUS:
      activeBlock->emit(Int{0});
      compileExpression(ast->child[0]);
      activeBlock->emit(Sub{});
      return;
    case ZEND_AST_UNARY_PLUS:
      activeBlock->emit(Int{0});
      compileExpression(ast->child[0]);
      activeBlock->emit(Add{});
      return;
    case ZEND_AST_UNARY_OP:
      compileExpression(ast->child[0]);
      switch(ast->attr) {
        case ZEND_BOOL_NOT:
          activeBlock->emit(Not{});
          return;
        case ZEND_BW_NOT:
          activeBlock->emit(BitNot{});
          return;
      }
  }
  panic("unknown unop");
}

void Compiler::compileExpression(const zend_ast* ast) {
  switch (ast->kind) {
    case ZEND_AST_ZVAL:
      compileZvalLiteral(zend_ast_get_zval(ast));
      break;
    case ZEND_AST_CONST:
      compileConstant(ast);
      break;
    case ZEND_AST_UNARY_MINUS:
    case ZEND_AST_UNARY_PLUS:
    case ZEND_AST_UNARY_OP:
      compileUnaryOp(ast);
      break;
    case ZEND_AST_BINARY_OP:
    case ZEND_AST_GREATER:
    case ZEND_AST_GREATER_EQUAL:
      compileExpression(ast->child[0]);
      compileExpression(ast->child[1]);
      activeBlock->emit(opForBinaryOp(ast));
      break;
    default:
      panic("unsupported expression");
  }
}

void Compiler::compileStatement(const zend_ast* ast) {
  switch (ast->kind) {
    case ZEND_AST_STMT_LIST: {
      // just a block, so recur please :)
      auto list = zend_ast_get_list(ast);
      for (uint32_t i = 0; i < list->children; i++) {
        compileStatement(list->child[i]);
      }
      break;
    }
    case ZEND_AST_ECHO:
      compileExpression(ast->child[0]);
      activeBlock->emit(Print{});
      activeBlock->emit(PopC{});
      break;
    case ZEND_AST_IF:
      compileIf(ast);
      break;
    default:
      panic("unsupported statement");
  }
}

void Compiler::compileIf(const zend_ast* ast) {
  auto list = zend_ast_get_list(ast);

  // where control will return after we're finished
  auto end = activeFunction->allocateBlock();

  for (uint32_t i = 0; i <list->children; i++) {
    auto elem = list->child[i];
    auto condition = elem->child[0];
    auto contents = elem->child[1];
    if (!condition) {
      // if no condition is provided, this is the 'else' branch
      compileStatement(contents);
      break;
    } else {
      // otherwise, we have a conditional
      auto mainBlock = activeBlock;
      auto branch = activeFunction->allocateBlock();

      activeBlock = branch;
      compileStatement(contents);
      activeBlock->exit(Jmp{end});
      activeBlock = mainBlock;

      compileExpression(condition);
      branchTo<JmpNZ>(branch);
    }
  }

  activeBlock->exit(Jmp{end});
  activeBlock = end;

}

}} // HPHP::php7
