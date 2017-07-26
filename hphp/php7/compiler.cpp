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

inline const zend_ast_decl* zend_ast_get_decl(const zend_ast* ast) {
  return reinterpret_cast<const zend_ast_decl*>(ast);
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

Compiler::Compiler(const std::string& filename) {
  unit = std::make_unique<Unit>();
  unit->name = filename;
}

void Compiler::compileProgram(const zend_ast* ast) {
  assert(ast->kind == ZEND_AST_STMT_LIST);

  auto pseudomain = unit->getPseudomain();
  activeFunction = pseudomain;
  activeBlock = pseudomain->entry;

  compileStatement(ast);

  if (activeBlock) {
    activeBlock->emit(Int{1});
    activeBlock->exit(RetC{});
  }
}

void Compiler::compileFunction(const zend_ast* ast) {
  auto decl = zend_ast_get_decl(ast);
  auto name = decl->name;
  auto params = zend_ast_get_list(decl->child[0]);
  auto body = decl->child[2];

  auto func = unit->makeFunction(ZSTR_VAL(name));
  std::swap(activeFunction, func);
  SCOPE_EXIT { std::swap(activeFunction, func); };

  if (decl->flags & ZEND_ACC_RETURN_REFERENCE) {
    activeFunction->attr |= Attr::AttrReference;
  }
  activeFunction->params.reserve(params->children);
  for (uint32_t i = 0; i < params->children; i++) {
    auto param = params->child[i];
    auto param_name = zval_to_string(zend_ast_get_zval(param->child[1]));
    auto param_default = param->child[2];
    auto param_attrs = param->attr;

    if (param_default) {
      panic("default parameter values not supported");
    }

    activeFunction->params.push_back({
      param_name,
      static_cast<bool>(param_attrs & ZEND_PARAM_REF)
    });
  }

  withBlock(activeFunction->entry, [&] {
    activeBlock = activeFunction->entry;
    compileStatement(body);

    if (activeBlock) {
      activeBlock->emit(Null{});
      activeBlock->exit(RetC{});
    }
  });
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

void Compiler::compileVar(const zend_ast* ast, Destination dest) {
  switch (dest.flavor) {
    case Drop:
      getLvalue(ast)->getC();
      activeBlock->emit(PopC{});
      return;
    case Cell:
      getLvalue(ast)->getC();
      return;
    case Ref:
      getLvalue(ast)->getV();
      return;
    case FuncParam:
      getLvalue(ast)->getF(dest.slot);
      return;
    case Return:
      panic("Can't get a local with R flavor");
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
      compileExpression(ast->child[0], Flavor::Cell);
      activeBlock->emit(Sub{});
      return;
    case ZEND_AST_UNARY_PLUS:
      activeBlock->emit(Int{0});
      compileExpression(ast->child[0], Flavor::Cell);
      activeBlock->emit(Add{});
      return;
    case ZEND_AST_UNARY_OP:
      compileExpression(ast->child[0], Flavor::Cell);
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
  auto lhs = ast->child[0];
  auto rhs = ast->child[1];

  getLvalue(lhs)->assign(rhs);
}

void Compiler::compileBind(const zend_ast* ast) {
  auto lhs = ast->child[0];
  auto rhs = ast->child[1];

  getLvalue(lhs)->bind(rhs);
}

void Compiler::compileAssignOp(const zend_ast* ast) {
  auto rhs = ast->child[1];
  auto op = getSetOpOp(ast->attr);
  auto lhs = ast->child[0];

  getLvalue(lhs)->assignOp(op, rhs);
}

void Compiler::compileCall(const zend_ast* ast) {
  auto callee = ast->child[0];
  auto params = zend_ast_get_list(ast->child[1]);

  auto zv = callee->kind == ZEND_AST_ZVAL
      ? zend_ast_get_zval(callee)
      : nullptr;

  // push the FPI record
  if (zv && Z_TYPE_P(zv) == IS_STRING) {
    activeBlock->emit(FPushFuncD{
      params->children,
      Z_STRVAL_P(zend_ast_get_zval(callee))
    });
  } else {
    compileExpression(callee, Flavor::Cell);
    activeBlock->emit(FPushFunc{
      params->children
    });
  }

  for (uint32_t i = 0; i < params->children; i++) {
    compileExpression(params->child[i], Destination::Param(i));
  }

  activeBlock->emit(FCall{params->children});
}

void Compiler::compileArray(const zend_ast* ast) {
  // TODO: handle static array literals, too
  auto list = zend_ast_get_list(ast);

  // NB: array() and list() share the syntax node
  // ZEND_AST_ARRAY--hence this odd check
  if (list->attr == ZEND_ARRAY_SYNTAX_LIST) {
    throw LanguageException("Cannot use list() as a standalone expression");
  }

  activeBlock->emit(NewArray{list->children});
  for (uint32_t i = 0; i < list->children; i++) {
    auto item = list->child[i];

    if (!item) {
      throw LanguageException("Cannot use empty array elements in arrays");
    }

    // NB: there's actually no constant for this in the parser; it's currently
    // just set to 1 for ref-y items
    bool ref = item->attr != 0;
    auto flavor = ref ? Flavor::Ref : Flavor::Cell;

    // if the second child is set, this is a name-value pair
    if (item->child[1]) {
      auto key = item->child[1];
      auto val = item->child[0];
      compileExpression(key, Flavor::Cell);
      compileExpression(val, flavor);
      if (ref) {
        activeBlock->emit(AddElemV{});
      } else {
        activeBlock->emit(AddElemC{});
      }
    } else {
      auto val = item->child[0];
      compileExpression(val, flavor);
      if (ref) {
        activeBlock->emit(AddNewElemV{});
      } else {
        activeBlock->emit(AddNewElemC{});
      }
    }
  }
}

void Compiler::compileExpression(const zend_ast* ast, Destination dest) {
  switch (ast->kind) {
    case ZEND_AST_ZVAL:
      compileZvalLiteral(zend_ast_get_zval(ast));
      fixFlavor(dest, Flavor::Cell);
      break;
    case ZEND_AST_CONST:
      compileConstant(ast);
      fixFlavor(dest, Flavor::Cell);
      break;
    case ZEND_AST_UNARY_MINUS:
    case ZEND_AST_UNARY_PLUS:
    case ZEND_AST_UNARY_OP:
      compileUnaryOp(ast);
      fixFlavor(dest, Flavor::Cell);
      break;
    case ZEND_AST_BINARY_OP:
    case ZEND_AST_GREATER:
    case ZEND_AST_GREATER_EQUAL:
      compileExpression(ast->child[0], Flavor::Cell);
      compileExpression(ast->child[1], Flavor::Cell);
      activeBlock->emit(opForBinaryOp(ast));
      fixFlavor(dest, Flavor::Cell);
      break;
    case ZEND_AST_POST_INC:
    case ZEND_AST_POST_DEC:
    case ZEND_AST_PRE_INC:
    case ZEND_AST_PRE_DEC:
      compileIncDec(ast);
      fixFlavor(dest, Flavor::Cell);
      break;
    case ZEND_AST_VAR:
      compileVar(ast, dest);
      break;
    case ZEND_AST_ASSIGN:
      compileAssignment(ast);
      fixFlavor(dest, Flavor::Cell);
      break;
    case ZEND_AST_ASSIGN_REF:
      compileBind(ast);
      fixFlavor(dest, Flavor::Ref);
      break;
    case ZEND_AST_ASSIGN_OP:
      compileAssignOp(ast);
      fixFlavor(dest, Flavor::Cell);
      break;
    case ZEND_AST_CALL:
      compileCall(ast);
      fixFlavor(dest, Flavor::Return);
      break;
    case ZEND_AST_ARRAY:
      compileArray(ast);
      fixFlavor(dest, Flavor::Cell);
      break;
    default:
      panic("unsupported expression");
  }
}

void Compiler::fixFlavor(Destination dest, Flavor actual) {
  switch(dest.flavor) {
    case Drop:
      switch (actual) {
        case Drop:
          return;
        case Cell:
          activeBlock->emit(PopC{});
          return;
        case Ref:
          activeBlock->emit(PopV{});
          return;
        case Return:
          activeBlock->emit(PopR{});
          return;
        case FuncParam:
          panic("Can't drop param");
          return;
      }
    case Ref:
      switch (actual) {
        case Cell:
          activeBlock->emit(Box{});
          return;
        case Ref:
          return;
        case Return:
          activeBlock->emit(BoxR{});
          return;
        case Drop:
        case FuncParam:
          panic("Can't make ref");
      }
    case Cell:
      switch (actual) {
        case Cell:
          return;
        case Ref:
          activeBlock->emit(Unbox{});
          return;
        case Return:
          activeBlock->emit(UnboxR{});
          return;
        case Drop:
        case FuncParam:
          panic("Can't make cell");
      }
    case Return:
      panic("can't coerce to return value");
      break;
    case FuncParam:
      auto slot = dest.slot;
      switch (actual) {
        case Cell:
          activeBlock->emit(FPassCE{slot});
          return;
        case Ref:
          activeBlock->emit(FPassV{slot});
          return;
        case Return:
          activeBlock->emit(FPassR{slot});
          return;
        case Drop:
        case FuncParam:
          panic("Can't make function param");
      }
  }
}

void Compiler::compileStatement(const zend_ast* ast) {
  switch (ast->kind) {
    case ZEND_AST_STMT_LIST: {
      // just a block, so recur please :)
      auto list = zend_ast_get_list(ast);
      for (uint32_t i = 0; i < list->children; i++) {
        if (!list->child[i]) {
          continue;
        }
        compileStatement(list->child[i]);
      }
      break;
    }
    case ZEND_AST_ECHO:
      compileExpression(ast->child[0], Flavor::Cell);
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
    case ZEND_AST_FOR:
      compileFor(ast);
      break;
    case ZEND_AST_BREAK:
      activeBlock->exit(Jmp{currentLoop("break").continuation});
      activeBlock = nullptr;
      break;
    case ZEND_AST_GLOBAL:
      compileGlobalDeclaration(ast);
      break;
    case ZEND_AST_CONTINUE:
      activeBlock->exit(Jmp{currentLoop("continue").test});
      activeBlock = nullptr;
      break;
    case ZEND_AST_RETURN:
      if (!ast->child[0]) {
        activeBlock->emit(Null{});
        activeBlock->exit(RetC{});
      } else if (activeFunction->attr & Attr::AttrReference) {
        compileExpression(ast->child[0], Flavor::Ref);
        activeBlock->exit(RetV{});
      } else {
        compileExpression(ast->child[0], Flavor::Cell);
        activeBlock->exit(RetC{});
      }
      activeBlock = nullptr;
      break;
    case ZEND_AST_FUNC_DECL:
      compileFunction(ast);
      break;
    default:
      compileExpression(ast, Flavor::Drop);
  }
}

void Compiler::compileGlobalDeclaration(const zend_ast* ast) {
  auto var = ast->child[0];
  auto zv = var->child[0];
  auto name = zval_to_string(zend_ast_get_zval(zv));

  activeBlock->emit(String{name});
  activeBlock->emit(VGetG{});
  activeBlock->emit(BindL{name});
  activeBlock->emit(PopV{});
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
        if (activeBlock) {
          activeBlock->exit(Jmp{end});
        }
      });

      compileExpression(condition, Flavor::Cell);
      branchTo<JmpNZ>(branch);
    }
  }

  if (activeBlock) {
    activeBlock->exit(Jmp{end});
  }
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
    if (activeBlock) {
      activeBlock->exit(Jmp{testBlock});
    }
    activeLoops.pop();
  });

  withBlock(testBlock, [&]() {
    compileExpression(condition, Flavor::Cell);
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

void Compiler::compileFor(const zend_ast* ast) {
  auto initializers = zend_ast_get_list(ast->child[0]);
  auto tests = zend_ast_get_list(ast->child[1]);
  auto increments = zend_ast_get_list(ast->child[2]);
  auto body = ast->child[3];

  // compile the initializers
  if (initializers) {
    for (uint32_t i = 0; i < initializers->children; i++) {
      compileExpression(initializers->child[i], Flavor::Drop);
    }
  }

  auto continuation = activeFunction->allocateBlock();
  auto bodyBlock = activeFunction->allocateBlock();
  auto testBlock = activeFunction->allocateBlock();
  auto incrementBlock = activeFunction->allocateBlock();

  withBlock(bodyBlock, [&]() {
    activeLoops.push({continuation, incrementBlock});
    compileStatement(body);
    if (activeBlock) {
      activeBlock->exit(Jmp{incrementBlock});
    }
    activeLoops.pop();
  });

  withBlock(testBlock, [&]() {
    // an empty condition list is the same as one that is always successful
    if (tests) {
      for (uint32_t i = 0; i < tests->children; i++) {
        // ignore all but the last test
        auto flavor = (i + 1 < tests->children)
          ? Flavor::Drop
          : Flavor::Cell;
        compileExpression(tests->child[i], flavor);
      }
      activeBlock->exit(JmpZ{continuation});
    }

    activeBlock->exit(Jmp{bodyBlock});
  });

  withBlock(incrementBlock, [&]() {
    if (increments) {
      for (uint32_t i = 0; i < increments->children; i++) {
        compileExpression(increments->child[i], Flavor::Drop);
      }
    }
    activeBlock->exit(Jmp{testBlock});
  });

  activeBlock->exit(Jmp{testBlock});
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

  void getV() override {
    c.activeFunction->locals.insert(name);
    c.activeBlock->emit(VGetL{name});
  }

  void getF(uint32_t slot) override {
    c.activeFunction->locals.insert(name);
    c.activeBlock->emit(FPassL{slot, name});
  }

  void assign(const zend_ast* rhs) override {
    c.activeFunction->locals.insert(name);
    c.compileExpression(rhs, Flavor::Cell);
    c.activeBlock->emit(SetL{name});
  }

  void bind(const zend_ast* rhs) override {
    c.activeFunction->locals.insert(name);
    c.compileExpression(rhs, Flavor::Ref);
    c.activeBlock->emit(BindL{name});
  }

  void assignOp(SetOpOp op, const zend_ast* rhs) override {
    c.activeFunction->locals.insert(name);
    c.compileExpression(rhs, Flavor::Cell);
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

  void getName() {
    c.compileExpression(nameExpr, Flavor::Cell);
  }

  void getC() override {
    getName();
    c.activeBlock->emit(CGetN{});
  }

  void getV() override {
    getName();
    c.activeBlock->emit(VGetN{});
  }

  void getF(uint32_t slot) override {
    getName();
    c.activeBlock->emit(FPassN{slot});
  }

  void assign(const zend_ast* rhs) override {
    getName();
    c.compileExpression(rhs, Flavor::Cell);
    c.activeBlock->emit(SetN{});
  }

  void bind(const zend_ast* rhs) override {
    getName();
    c.compileExpression(rhs, Flavor::Ref);
    c.activeBlock->emit(BindN{});
  }

  void assignOp(SetOpOp op, const zend_ast* rhs) override {
    getName();
    c.compileExpression(rhs, Flavor::Cell);
    c.activeBlock->emit(SetOpN{op});
  }

  void incDec(IncDecOp op) override {
    getName();
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
