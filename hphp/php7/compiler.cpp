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

inline std::string zval_to_string(const zval* zv) {
  std::string out;
  switch (Z_TYPE_P(zv)) {
    case IS_LONG:
      folly::format(&out, "{}", Z_LVAL_P(zv));
      break;
    case IS_NULL:
    case IS_FALSE:
      break;
    case IS_TRUE:
      out.append("1");
      break;
    case IS_DOUBLE:
      folly::format(&out, "{}", Z_DVAL_P(zv));
      break;
    case IS_STRING:
      folly::format(&out, "{}", Z_STRVAL_P(zv));
      break;
  }
  return out;
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

void Compiler::compileVar(const zend_ast* ast) {
  getLvalue(ast)->getC();
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

IncDecOp Compiler::getIncDecOpForNode(zend_ast_kind kind) {
  switch (kind) {
    case ZEND_AST_PRE_INC:
      return IncDecOp::PreInc;
    case ZEND_AST_PRE_DEC:
      return IncDecOp::PreDec;
    case ZEND_AST_POST_INC:
      return IncDecOp::PostInc;
    case ZEND_AST_POST_DEC:
      return IncDecOp::PostDec;
    default:
      panic("not a inc/dec node");
  }
}

SetOpOp Compiler::getSetOpOp(zend_ast_attr attr) {
  switch (attr) {
    case ZEND_ASSIGN_ADD:
      return SetOpOp::PlusEqual;
    case ZEND_ASSIGN_SUB:
      return SetOpOp::MinusEqual;
    case ZEND_ASSIGN_MUL:
      return SetOpOp::MulEqual;
    case ZEND_ASSIGN_POW:
      return SetOpOp::PowEqual;
    case ZEND_ASSIGN_DIV:
      return SetOpOp::DivEqual;
    case ZEND_ASSIGN_CONCAT:
      return SetOpOp::ConcatEqual;
    case ZEND_ASSIGN_MOD:
      return SetOpOp::ModEqual;
    case ZEND_ASSIGN_BW_AND:
      return SetOpOp::AndEqual;
    case ZEND_ASSIGN_BW_OR:
      return SetOpOp::OrEqual;
    case ZEND_ASSIGN_BW_XOR:
      return SetOpOp::XorEqual;
    case ZEND_ASSIGN_SL:
      return SetOpOp::SlEqual;
    case ZEND_ASSIGN_SR:
      return SetOpOp::SrEqual;
    default:
      panic("unsupported set-op");
  }
}

void Compiler::compileIncDec(const zend_ast* ast) {
  auto op = getIncDecOpForNode(ast->kind);
  auto var = ast->child[0];

  getLvalue(var)->incDec(op);
}

void Compiler::compileAssignment(const zend_ast* ast) {
  auto rhs = ast->child[1];
  auto lhs = ast->child[0];

  getLvalue(lhs)->assign(rhs);
}

void Compiler::compileAssignOp(const zend_ast* ast) {
  auto rhs = ast->child[1];
  auto op = getSetOpOp(ast->attr);
  auto lhs = ast->child[0];

  getLvalue(lhs)->assignOp(op, rhs);
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
    case ZEND_AST_POST_INC:
    case ZEND_AST_POST_DEC:
    case ZEND_AST_PRE_INC:
    case ZEND_AST_PRE_DEC:
      compileIncDec(ast);
      break;
    case ZEND_AST_VAR:
      compileVar(ast);
      break;
    case ZEND_AST_ASSIGN:
      compileAssignment(ast);
      break;
    case ZEND_AST_ASSIGN_OP:
      compileAssignOp(ast);
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
    case ZEND_AST_WHILE:
      compileWhile(ast->child[0], ast->child[1], false);
      break;
    case ZEND_AST_DO_WHILE:
      compileWhile(ast->child[1], ast->child[0], true);
      break;
    default:
      compileExpression(ast);
      activeBlock->emit(PopC{});
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
      auto branch = activeFunction->allocateBlock();

      withBlock(branch, [&]() {
        compileStatement(contents);
        activeBlock->exit(Jmp{end});
      });

      compileExpression(condition);
      branchTo<JmpNZ>(branch);
    }
  }

  activeBlock->exit(Jmp{end});
  activeBlock = end;
}

void Compiler::compileWhile(const zend_ast* condition, const zend_ast* body,
                            bool bodyFirst) {

  auto continuation = activeFunction->allocateBlock();
  auto bodyBlock = activeFunction->allocateBlock();
  auto testBlock = activeFunction->allocateBlock();

  withBlock(bodyBlock, [&]() {
    activeLoops.push({continuation, testBlock});
    compileStatement(body);
    activeBlock->exit(Jmp{testBlock});
    activeLoops.pop();
  });

  withBlock(testBlock, [&]() {
    compileExpression(condition);
    activeBlock->exit(JmpNZ{bodyBlock});
    activeBlock->exit(Jmp{continuation});
  });

  if (bodyFirst) {
    activeBlock->exit(Jmp{bodyBlock});
  } else {
    activeBlock->exit(Jmp{testBlock});
  }

  activeBlock = continuation;
}

struct Compiler::LocalLvalue : Compiler::Lvalue {
  LocalLvalue(Compiler& c, std::string name)
    : c(c)
    , name(std::move(name)) {}

  void getC() override {
    c.activeFunction->locals.insert(name);
    c.activeBlock->emit(CGetL{name});
  }

  void assign(const zend_ast* rhs) override {
    c.activeFunction->locals.insert(name);
    c.compileExpression(rhs);
    c.activeBlock->emit(SetL{name});
  }

  void assignOp(SetOpOp op, const zend_ast* rhs) override {
    c.activeFunction->locals.insert(name);
    c.compileExpression(rhs);
    c.activeBlock->emit(SetOpL{name, op});
  }

  void incDec(IncDecOp op) override {
    c.activeFunction->locals.insert(name);
    c.activeBlock->emit(IncDecL{name, op});
  }

  Compiler& c;
  std::string name;
};

struct Compiler::DynamicLocalLvalue : Compiler::Lvalue {
  DynamicLocalLvalue(Compiler& c, const zend_ast* nameExpr)
    : c(c)
    , nameExpr(nameExpr) {}

  void getC() override {
    c.compileExpression(nameExpr);
    c.activeBlock->emit(CGetN{});
  }

  void assign(const zend_ast* rhs) override {
    c.compileExpression(nameExpr);
    c.compileExpression(rhs);
    c.activeBlock->emit(SetN{});
  }

  void assignOp(SetOpOp op, const zend_ast* rhs) override {
    c.compileExpression(nameExpr);
    c.compileExpression(rhs);
    c.activeBlock->emit(SetOpN{op});
  }

  void incDec(IncDecOp op) override {
    c.compileExpression(nameExpr);
    c.activeBlock->emit(IncDecN{op});
  }

  Compiler& c;
  const zend_ast* nameExpr;
};

std::unique_ptr<Compiler::Lvalue> Compiler::getLvalue(const zend_ast* ast) {
  if (ast->kind == ZEND_AST_VAR) {
    auto name = ast->child[0];
    switch (name->kind) {
      case ZEND_AST_ZVAL:
        return std::make_unique<Compiler::LocalLvalue>(
            *this,
            zval_to_string(zend_ast_get_zval(name)));
      default:
        return std::make_unique<Compiler::DynamicLocalLvalue>(
            *this,
            name);
    }
  }

  panic("unsupported lvalue");
}

}} // HPHP::php7
