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

#include "hphp/php7/util.h"
#include "hphp/util/match.h"

#include <folly/Format.h>

#include <iostream>

namespace HPHP { namespace php7 {

namespace {

void buildFunction(Function* func, const zend_ast* ast);

CFG compileZvalLiteral(const zval* ast);
CFG compileConstant(const zend_ast* ast);
CFG compileVar(const zend_ast* ast, Destination destination);
CFG compileAssignment(const zend_ast* ast);
CFG compileBind(const zend_ast* ast);
CFG compileAssignOp(const zend_ast* ast);
CFG compileFunctionCall(const zend_ast* ast);
CFG compileMethodCall(const zend_ast_list* params);
CFG compileNew(const zend_ast* ast);
CFG compileCall(const zend_ast_list* params);
CFG compileArray(const zend_ast* ast);

CFG compileGlobalDeclaration(const zend_ast* ast);
CFG compileCatch(Function* func, const zend_ast_list* catches);
CFG compileTry(Function* func, const zend_ast* ast);
CFG compileIf(Function* func, const zend_ast* ast);
CFG compileWhile(Function* func,
    const zend_ast* cond,
    const zend_ast* body,
    bool bodyFirst);
CFG compileFor(Function* func, const zend_ast* ast);

Bytecode opForBinaryOp(const zend_ast* op);
IncDecOp getIncDecOpForNode(zend_ast_kind kind);
SetOpOp getSetOpOp(zend_ast_attr attr);

std::unique_ptr<Lvalue> getLvalue(const zend_ast* ast);

CFG compileUnaryOp(const zend_ast* op);
CFG compileIncDec(const zend_ast* op);

CFG fixFlavor(Destination dest, Flavor actual);

Attr getMemberModifiers(uint32_t flags);
void compileClassStatement(Class* cls, const zend_ast* ast);
void compileMethod(Class* cls, const zend_ast* ast);

} // namespace

using namespace bc;

void compileProgram(Unit* unit, const zend_ast* ast) {
  assert(ast->kind == ZEND_AST_STMT_LIST);

  auto pseudomain = unit->getPseudomain();

  pseudomain->cfg = compileStatement(pseudomain, ast)
    .then(Int{1})
    .thenReturn(Flavor::Cell)
    .makeExitsReal()
    .inRegion(std::make_unique<Region>(Region::Kind::Entry));
}

void compileFunction(Unit* unit, const zend_ast* ast) {
  buildFunction(unit->makeFunction(), ast);
}

CFG compileClass(Unit* unit, const zend_ast* ast) {
  auto decl = zend_ast_get_decl(ast);
  auto name = ZSTR_VAL(decl->name);
  auto attr = decl->flags;
  auto statements = zend_ast_get_list(decl->child[2]);

  auto cls = unit->makeClass();
  cls->name = name;

  if (attr & ZEND_ACC_EXPLICIT_ABSTRACT_CLASS) {
    cls->attr |= Attr::AttrAbstract;
  }
  if (attr & ZEND_ACC_FINAL) {
    cls->attr |= Attr::AttrFinal;
  }

  for (uint32_t i = 0; i < statements->children; i++) {
    compileClassStatement(cls, statements->child[i]);
  }

  return CFG(DefCls{cls->index});
}

void panic(const std::string& msg) {
  throw CompilerException(folly::sformat("panic: {}", msg));
}

namespace {

void buildFunction(Function* func, const zend_ast* ast) {
  auto decl = zend_ast_get_decl(ast);
  auto name = ZSTR_VAL(decl->name);
  auto params = zend_ast_get_list(decl->child[0]);
  auto body = decl->child[2];

  func->name = name;

  if (decl->flags & ZEND_ACC_RETURN_REFERENCE) {
    func->attr |= Attr::AttrReference;
  }

  func->params.reserve(params->children);
  for (uint32_t i = 0; i < params->children; i++) {
    auto param = params->child[i];
    auto param_name = zval_to_string(zend_ast_get_zval(param->child[1]));
    auto param_default = param->child[2];
    auto param_attrs = param->attr;

    if (param_default) {
      panic("default parameter values not supported");
    }

    func->params.push_back({
      param_name,
      static_cast<bool>(param_attrs & ZEND_PARAM_REF)
    });
  }

  func->cfg = compileStatement(func, body)
    .then(Null{})
    .thenReturn(func->returnsByReference()
      ? Flavor::Ref
      : Flavor::Cell)
    .makeExitsReal()
    .inRegion(std::make_unique<Region>(Region::Kind::Entry));
}

CFG compileZvalLiteral(const zval* zv) {
  switch (Z_TYPE_P(zv)) {
    case IS_LONG:
      return { Int{Z_LVAL_P(zv)} };
    case IS_NULL:
      return { Null{} };
    case IS_FALSE:
      return { False{} };
    case IS_TRUE:
      return { True{} };
    case IS_DOUBLE:
      return { Double{Z_DVAL_P(zv)} };
    case IS_STRING:
      return { String{Z_STRVAL_P(zv)} };
    default:
      panic("unsupported literal");
  }
}

CFG compileConstant(const zend_ast* ast) {
  auto name = ast->child[0];
  const char* str = Z_STRVAL_P(zend_ast_get_zval(name));
  if (name->attr & ZEND_NAME_NOT_FQ) {
    if (strcasecmp(str, "true") == 0) {
      return { True{} };
    } else if (strcasecmp(str, "false") == 0) {
      return { False{} };
    } else if (strcasecmp(str, "null") == 0) {
      return { Null{} };
    } else {
      panic("unknown unqualified constant");
    }
  } else {
    panic("unknown constant");
  }
}


std::unique_ptr<Lvalue> getLvalue(const zend_ast* ast){
  if (auto ret = Lvalue::getLvalue(ast)) {
    return ret;
  }

  panic("can't make lvalue");
}

CFG compileVar(const zend_ast* ast, Destination dest) {
  switch (dest.flavor) {
    case Drop:
      return getLvalue(ast)->getC()
        .then(PopC{});
    case Cell:
      return getLvalue(ast)->getC();
    case Ref:
      return getLvalue(ast)->getV();
    case FuncParam:
      return getLvalue(ast)->getF(dest.slot);
    case Return:
      panic("Can't get a local with R flavor");
  }
  panic("bad destination flavor");
}

Bytecode opForBinaryOp(const zend_ast* ast) {
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

CFG compileUnaryOp(const zend_ast* ast) {
  switch (ast->kind) {
    case ZEND_AST_UNARY_MINUS:
      return CFG()
        .then(Int{0})
        .then(compileExpression(ast->child[0], Flavor::Cell))
        .then(Sub{});
    case ZEND_AST_UNARY_PLUS:
      return CFG()
        .then(Int{0})
        .then(compileExpression(ast->child[0], Flavor::Cell))
        .then(Add{});
    case ZEND_AST_UNARY_OP:
      return CFG()
        .then(compileExpression(ast->child[0], Flavor::Cell))
        .then(ast->attr == ZEND_BOOL_NOT
            ? Bytecode{Not{}}
            : Bytecode(BitNot{}));
  }
  panic("unknown unop");
}

IncDecOp getIncDecOpForNode(zend_ast_kind kind) {
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

SetOpOp getSetOpOp(zend_ast_attr attr) {
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

CFG compileIncDec(const zend_ast* ast) {
  auto op = getIncDecOpForNode(ast->kind);
  auto var = ast->child[0];

  return getLvalue(var)->incDec(op);
}

CFG compileAssignment(const zend_ast* ast) {
  auto lhs = ast->child[0];
  auto rhs = ast->child[1];

  return getLvalue(lhs)->assign(rhs);
}

CFG compileBind(const zend_ast* ast) {
  auto lhs = ast->child[0];
  auto rhs = ast->child[1];

  return getLvalue(lhs)->bind(rhs);
}

CFG compileAssignOp(const zend_ast* ast) {
  auto rhs = ast->child[1];
  auto op = getSetOpOp(ast->attr);
  auto lhs = ast->child[0];

  return getLvalue(lhs)->assignOp(op, rhs);
}

CFG compileFunctionCall(const zend_ast* ast) {
  auto callee = ast->child[0];
  auto params = zend_ast_get_list(ast->child[1]);

  auto zv = callee->kind == ZEND_AST_ZVAL
      ? zend_ast_get_zval(callee)
      : nullptr;

  if (zv && Z_TYPE_P(zv) == IS_STRING) {
    return CFG(FPushFuncD{
      params->children,
      Z_STRVAL_P(zend_ast_get_zval(callee))
    }).then(compileCall(params));
  } else {
    return compileExpression(callee, Flavor::Cell)
      .then(FPushFunc{
        params->children
      })
      .then(compileCall(params));
  }
}

CFG compileMethodCall(const zend_ast* ast) {
  auto target = ast->child[0];
  auto method = ast->child[1];
  auto params = zend_ast_get_list(ast->child[2]);

  auto zv = method->kind == ZEND_AST_ZVAL
      ? zend_ast_get_zval(method)
      : nullptr;

  auto targetCfg = compileExpression(target, Flavor::Cell);

  if (zv && Z_TYPE_P(zv) == IS_STRING) {
    return targetCfg
      .then(FPushObjMethodD{
        params->children,
        Z_STRVAL_P(zv),
        ObjMethodOp::NullThrows
      })
      .then(compileCall(params));
  } else {
    return compileExpression(method, Flavor::Cell)
      .then(std::move(targetCfg))
      .then(FPushObjMethod{
        params->children,
        ObjMethodOp::NullThrows
      });
  }
}

CFG compileNew(const zend_ast* ast) {
  auto cls = ast->child[0];
  auto params = zend_ast_get_list(ast->child[1]);

  auto zv = cls->kind == ZEND_AST_ZVAL
      ? zend_ast_get_zval(cls)
      : nullptr;

  // push the FPI record
  if (zv && Z_TYPE_P(zv) == IS_STRING) {
    return CFG(FPushCtorD{
      params->children,
      Z_STRVAL_P(zv)
    })
      .then(compileCall(params))
      .then(PopR{});
  } else {
    panic("new with variable classref");
  }
}

CFG compileCall(const zend_ast_list* params) {
  CFG call;
  for (uint32_t i = 0; i < params->children; i++) {
    call.then(compileExpression(params->child[i], Destination::Param(i)));
  }

  return call.then(FCall{params->children});
}

CFG compileArray(const zend_ast* ast) {
  // TODO: handle static array literals, too
  auto list = zend_ast_get_list(ast);

  // NB: array() and list() share the syntax node
  // ZEND_AST_ARRAY--hence this odd check
  if (list->attr == ZEND_ARRAY_SYNTAX_LIST) {
    throw LanguageException("Cannot use list() as a standalone expression");
  }

  CFG cfg(NewArray{list->children});
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
      cfg.then(compileExpression(key, Flavor::Cell))
         .then(compileExpression(val, flavor));
      if (ref) {
        cfg.then(AddElemV{});
      } else {
        cfg.then(AddElemC{});
      }
    } else {
      auto val = item->child[0];
      cfg.then(compileExpression(val, flavor));
      if (ref) {
        cfg.then(AddNewElemV{});
      } else {
        cfg.then(AddNewElemC{});
      }
    }
  }
  return cfg;
}

} // namespace

CFG compileExpression(const zend_ast* ast, Destination dest) {
  switch (ast->kind) {
    case ZEND_AST_ZVAL:
      return compileZvalLiteral(zend_ast_get_zval(ast))
        .then(fixFlavor(dest, Flavor::Cell));
    case ZEND_AST_CONST:
      return compileConstant(ast)
        .then(fixFlavor(dest, Flavor::Cell));
    case ZEND_AST_UNARY_MINUS:
    case ZEND_AST_UNARY_PLUS:
    case ZEND_AST_UNARY_OP:
      return compileUnaryOp(ast)
        .then(fixFlavor(dest, Flavor::Cell));
    case ZEND_AST_BINARY_OP:
    case ZEND_AST_GREATER:
    case ZEND_AST_GREATER_EQUAL:
      return CFG()
        .then(compileExpression(ast->child[0], Flavor::Cell))
        .then(compileExpression(ast->child[1], Flavor::Cell))
        .then(opForBinaryOp(ast))
        .then(fixFlavor(dest, Flavor::Cell));
    case ZEND_AST_POST_INC:
    case ZEND_AST_POST_DEC:
    case ZEND_AST_PRE_INC:
    case ZEND_AST_PRE_DEC:
      return compileIncDec(ast)
        .then(fixFlavor(dest, Flavor::Cell));
    case ZEND_AST_VAR:
    case ZEND_AST_DIM:
      return compileVar(ast, dest);
    case ZEND_AST_ASSIGN:
      return compileAssignment(ast)
        .then(fixFlavor(dest, Flavor::Cell));
    case ZEND_AST_ASSIGN_REF:
      return compileBind(ast)
        .then(fixFlavor(dest, Flavor::Ref));
      break;
    case ZEND_AST_ASSIGN_OP:
      return compileAssignOp(ast)
        .then(fixFlavor(dest, Flavor::Cell));
    case ZEND_AST_CALL:
      return compileFunctionCall(ast)
        .then(fixFlavor(dest, Flavor::Return));
    case ZEND_AST_METHOD_CALL:
      return compileMethodCall(ast)
        .then(fixFlavor(dest, Flavor::Return));
    case ZEND_AST_NEW:
      return compileNew(ast)
        .then(fixFlavor(dest, Flavor::Cell));
    case ZEND_AST_ARRAY:
      return compileArray(ast)
        .then(fixFlavor(dest, Flavor::Cell));
    default:
      panic("unsupported expression");
  }
}

namespace {

CFG fixFlavor(Destination dest, Flavor actual) {
  switch(dest.flavor) {
    case Drop:
      switch (actual) {
        case Drop:
          return {};
        case Cell:
          return { PopC{} };
        case Ref:
          return { PopV{} };
        case Return:
          return { PopR{} };
        case FuncParam:
          panic("Can't drop param");
      }
    case Ref:
      switch (actual) {
        case Cell:
          return { Box{} };
        case Ref:
          return {};
        case Return:
          return { BoxR{} };
        case Drop:
        case FuncParam:
          panic("Can't make ref");
      }
    case Cell:
      switch (actual) {
        case Cell:
          return {};
        case Ref:
          return { Unbox{} };
        case Return:
          return { UnboxR{} };
        case Drop:
        case FuncParam:
          panic("Can't make cell");
      }
    // nobody should be asking for a Return flavor unless they gosh darn know
    // the expression being compiled is a call >:/
    case Return:
      switch (actual) {
        case Return:
          return {};
        default:
          panic("can't coerce to return value");
      }
    case FuncParam:
      auto slot = dest.slot;
      switch (actual) {
        case Cell:
          return { FPassCE{slot} };
        case Ref:
          return { FPassV{slot} };
        case Return:
          return { FPassR{slot} };
        case Drop:
        case FuncParam:
          panic("Can't make function param");
      }
  }
  panic("bad destination flavor");
}

} // namespace

CFG compileStatement(Function* func, const zend_ast* ast) {
  switch (ast->kind) {
    case ZEND_AST_STMT_LIST: {
      // just a block, so recur please :)
      CFG cfg;
      auto list = zend_ast_get_list(ast);
      for (uint32_t i = 0; i < list->children; i++) {
        if (!list->child[i]) {
          continue;
        }
        cfg.then(compileStatement(func, list->child[i]));
      }
      return cfg;
    }
    case ZEND_AST_ECHO:
      return CFG()
        .then(compileExpression(ast->child[0], Flavor::Cell))
        .then(Print{})
        .then(PopC{});
    case ZEND_AST_IF:
      return compileIf(func, ast);
    case ZEND_AST_WHILE:
      return compileWhile(func, ast->child[0], ast->child[1], false);
    case ZEND_AST_DO_WHILE:
      return compileWhile(func, ast->child[1], ast->child[0], true);
    case ZEND_AST_FOR:
      return compileFor(func, ast);
    case ZEND_AST_BREAK:
      return CFG().thenBreak();
    case ZEND_AST_GLOBAL:
      return compileGlobalDeclaration(ast);
    case ZEND_AST_CONTINUE:
      return CFG().thenContinue();
    case ZEND_AST_RETURN: {
      auto flavor = func->returnsByReference()
        ? Flavor::Ref
        : Flavor::Cell;
      auto expr = ast->child[0]
        ? compileExpression(ast->child[0], flavor)
        : CFG(Null{}).then(fixFlavor(flavor, Flavor::Cell));
      return expr.thenReturn(flavor);
    }
    case ZEND_AST_TRY:
      return compileTry(func, ast);
    case ZEND_AST_THROW:
      return compileExpression(ast->child[0], Flavor::Cell)
        .thenThrow();
    case ZEND_AST_FUNC_DECL:
      compileFunction(func->parent, ast);
      return CFG();
    case ZEND_AST_CLASS:
      return compileClass(func->parent, ast);
    default:
      return compileExpression(ast, Flavor::Drop);
  }
}

namespace {

CFG compileGlobalDeclaration(const zend_ast* ast) {
  auto var = ast->child[0];
  auto zv = var->child[0];
  auto name = zval_to_string(zend_ast_get_zval(zv));

  return {
    String{name},
    VGetG{},
    BindL{NamedLocal{name}},
    PopV{}
  };
}

CFG compileCatch(Function* func, const zend_ast_list* catches) {
  CFG cfg;
  auto end = cfg.makeBlock();

  for (uint32_t i = 0; i < catches->children; i++) {
    auto clause = catches->child[i];
    auto types = zend_ast_get_list(clause->child[0]);
    auto capture = zval_to_string(zend_ast_get_zval(clause->child[1]));
    auto body = clause->child[2];

    auto handler = CFG({
      SetL{NamedLocal{capture}},
      PopC{}
    }).then(compileStatement(func, body))
      .thenJmp(end);

    for (uint32_t j = 0; j < types->children; j++) {
      auto name = zval_to_string(zend_ast_get_zval(types->child[j]));
      cfg.then(Dup{})
        .then(InstanceOfD{name})
        .branchNZ("handler");
    }

    cfg.replace("handler", std::move(handler));
  }

  cfg.thenThrow();

  return cfg.continueFrom(end);
}


CFG compileTry(Function* func, const zend_ast* ast) {
  auto body = ast->child[0];
  auto catches = zend_ast_get_list(ast->child[1]);
  auto finally = ast->child[2];

  if (!finally && catches->children == 0) {
    throw LanguageException("Cannot use try without catch or finally");
  }

  CFG cfg = compileStatement(func, body);

  // don't bother adding a catch region if there's no catches
  if (catches->children > 0) {
    cfg.addExnHandler(compileCatch(func, catches));
  }

  if (finally) {
    cfg.addFinallyGuard(compileStatement(func, finally));
  }

  return cfg;
}

CFG compileIf(Function* func, const zend_ast* ast) {
  auto list = zend_ast_get_list(ast);

  CFG cfg;
  auto end = cfg.makeBlock();

  for (uint32_t i = 0; i <list->children; i++) {
    auto elem = list->child[i];
    auto condition = elem->child[0];
    auto contents = elem->child[1];
    if (!condition) {
      // if no condition is provided, this is the 'else' branch
      cfg.then(compileStatement(func, contents));
    } else {
      cfg
        .then(compileExpression(condition, Flavor::Cell))
        .branchNZ(
          compileStatement(func, contents)
            .thenJmp(end)
        );
    }
  }

  cfg.thenJmp(end);

  return cfg.continueFrom(end);
}



CFG compileWhile(Function* func,
    const zend_ast* condition,
    const zend_ast* contents,
    bool bodyFirst) {
  return CFG::Labeled(
    ".start",
    CFG().then(bodyFirst
      ? ".loop-body"
      : ".loop-header"),

    ".loop-body",
    compileStatement(func, contents)
      .linkLoop(
        CFG().then(".end"),
        CFG().then(".loop-header"))
      .then(".loop-header"),

    ".loop-header",
    compileExpression(condition, Flavor::Cell)
      .branchNZ(".loop-body"),

    ".end",
    CFG()
  );

  return {};
}

CFG compileFor(Function* func, const zend_ast* ast) {
  auto initializers = zend_ast_get_list(ast->child[0]);
  auto tests = zend_ast_get_list(ast->child[1]);
  auto increments = zend_ast_get_list(ast->child[2]);
  auto body = ast->child[3];

  CFG cfg;

  const auto doAll = [&] (const zend_ast_list* list) {
    CFG all;
    if (list) {
      for (uint32_t i = 0; i < list->children; i++) {
        all.then(compileExpression(list->child[i], Flavor::Drop));
      }
    }
    return all;
  };

  CFG testCFG;
  if (tests) {
    for (uint32_t i = 0; i < tests->children; i++) {
      // ignore all but the last test
      auto flavor = (i + 1 < tests->children)
        ? Flavor::Drop
        : Flavor::Cell;
      testCFG.then(compileExpression(tests->child[i], flavor));
    }
    testCFG.branchNZ(".body");
  } else {
    testCFG.then(".body");
  }

  // compile the initializers
  cfg.then(CFG::Labeled(
    ".initializers",
    doAll(initializers)
      .then(".test"),

    ".body",
    compileStatement(func, body)
      .linkLoop(
        CFG().then(".end"),
        CFG().then(".increment"))
      .then(".increment"),

    ".increment",
    doAll(increments).then(".test"),

    ".test",
    std::move(testCFG),

    ".end",
    CFG()
  ));

  return cfg;
}

Attr getMemberModifiers(uint32_t flags) {
  Attr attr{};
  if (flags & ZEND_ACC_PUBLIC) {
    attr |= Attr::AttrPublic;
  }
  if (flags & ZEND_ACC_PROTECTED) {
    attr |= Attr::AttrProtected;
  }
  if (flags & ZEND_ACC_PRIVATE) {
    attr |= Attr::AttrPrivate;
  }
  if (flags & ZEND_ACC_STATIC) {
    attr |= Attr::AttrStatic;
  }
  if (flags & ZEND_ACC_ABSTRACT) {
    attr |= Attr::AttrAbstract;
  }
  if (flags & ZEND_ACC_FINAL) {
    attr |= Attr::AttrFinal;
  }
  return attr;
}

void compileClassStatement(Class* cls, const zend_ast* ast) {
  switch(ast->kind) {
    case ZEND_AST_METHOD:
      compileMethod(cls, ast);
      break;
    default:
      panic("unsupported class statement");
  }
}

void compileMethod(Class* cls, const zend_ast* ast) {
  auto func = cls->makeMethod();
  buildFunction(func, ast);
  func->attr |= getMemberModifiers(zend_ast_get_decl(ast)->flags);
}

} // namespace

}} // HPHP::php7
