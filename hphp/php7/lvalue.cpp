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

#include "hphp/php7/lvalue.h"

#include "hphp/php7/bytecode.h"
#include "hphp/php7/compiler.h"
#include "hphp/php7/util.h"

namespace HPHP { namespace php7 {

using namespace bc;

/* This corresponds to a sequence of member instructions in the output
 * bytecode. We need to track these separately since the member instruction
 * sequence is more or less treated as one unit: once a base pointer is live,
 * we can't push anything else to the stack until the base pointer is consumed.
 *
 * This class tracks elements that we push to the stack in advance (possibly for
 * member instructions) and generates the correct final member instruction
 */
struct BaseValue::MinstrSeq {
  explicit MinstrSeq(uint32_t slot)
    : isArg(true)
    , slot(slot) {}
  explicit MinstrSeq(MOpMode mode)
    : isArg(false)
    , mode(mode) {}

  void baseCell(uint32_t idx) {
    instrs.push_back(BaseC{idx});
  }
  void baseRet(uint32_t idx) {
    instrs.push_back(BaseR{idx});
  }

  void baseLocal(const bc::Local& l) {
    if (isArg) {
      instrs.push_back(FPassBaseL{slot, l});
    } else {
      instrs.push_back(BaseL{l, mode});
    }
  }

  void baseNamed(uint32_t depth) {
    if (isArg) {
      instrs.push_back(FPassBaseNC{slot, depth});
    } else {
      instrs.push_back(BaseNC{depth, mode});
    }
  }

  void dim(const bc::MemberKey& key) {
    if (isArg) {
      instrs.push_back(FPassDim{slot, key});
    } else {
      instrs.push_back(Dim{mode, key});
    }
  }

  /* Push a cell needed for the member instruction sequence
   * This returns the stack index you should use in the CellMember */
  uint32_t push() {
    keysOnStack++;
    return stackDepth++;
  }

  /* Push a cell not needed by the member instruction sequence but is an
   * argument to the final op, e.g. the RHS in an assignment or bind */
  uint32_t pushArgument() {
    return stackDepth++;
  }

  void finalQuery(const MemberKey& key, QueryMOp op) {
    instrs.push_back(QueryM{ keysOnStack, op, key});
  }

  void finalCGet(const MemberKey& key) {
    finalQuery(key, QueryMOp::CGet);
  }

  void finalFPass(const MemberKey& key) {
    assert(isArg);
    instrs.push_back(FPassM{ slot, keysOnStack, key});
  }

  void finalIsset(const MemberKey& key) {
    finalQuery(key, QueryMOp::Isset);
  }

  void finalEmpty(const MemberKey& key) {
    finalQuery(key, QueryMOp::Empty);
  }

  void finalVGet(const MemberKey& key) {
    instrs.push_back(VGetM{keysOnStack, key});
  }

  void finalSet(const MemberKey& key) {
    instrs.push_back(SetM{keysOnStack, key});
  }

  void finalSetOp(const MemberKey& key, SetOpOp op) {
    instrs.push_back(SetOpM{keysOnStack, op, key});
  }

  void finalBind(const MemberKey& key) {
    instrs.push_back(BindM{keysOnStack, key});
  }

  void finalIncDec(const MemberKey& key, IncDecOp op) {
    instrs.push_back(IncDecM{keysOnStack, op, key});
  }

  /* A visitor to fix the stack indices in member instructions
   * since they will be wrong
   * e.g.
   *   the last element pushed of 6 total will have index 5 but should have
   *   index 0
   */
  struct MemberKeyVisitor {
    explicit MemberKeyVisitor(uint32_t depth)
      : depth(depth) {}

    void fixIndex(uint32_t& location) {
      location = depth - location - 1;
    }

    void bytecode(BaseNC& b) { fixIndex(b.imm1); }
    void bytecode(BaseGC& b) { fixIndex(b.imm1); }
    void bytecode(BaseSC& b) { fixIndex(b.imm1); }
    void bytecode(BaseC& b) { fixIndex(b.imm1); }
    void bytecode(BaseR& b) { fixIndex(b.imm1); }
    void bytecode(FPassBaseNC& b) { fixIndex(b.imm2); }
    void bytecode(FPassBaseGC& b) { fixIndex(b.imm2); }

    template <class T>
    void bytecode(T& b) {
      b.visit_imms(*this);
    }

    void imm(MemberKey& key) {
      if (auto cell = boost::get<CellMember>(&key)) {
        fixIndex(cell->location);
      }
    }

    template <class T>
    void imm(T& /*imm*/) {}

    uint32_t depth;
  };

  void emit(Compiler& c) {
    for (auto& bc : instrs) {
      bc.visit(MemberKeyVisitor(stackDepth));
      c.activeBlock->emit(std::move(bc));
    }
  }

  bool isArg{false};
  uint32_t slot;
  MOpMode mode;
  std::vector<Bytecode> instrs;
  uint32_t stackDepth{0};
  uint32_t keysOnStack{0};
};

namespace {

/* An lvalue corresponding to a statically-known local
 * e.g.
 *   $x
 */
struct LocalLvalue : Lvalue {
  LocalLvalue(Compiler& c, std::string name)
    : Lvalue(c)
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

  void getB(MinstrSeq& m) override {
    c.activeFunction->locals.insert(name);
    m.baseLocal(Local{name});
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

  std::string name;
};

/* An lvalue corresponding to a local that is named dynamically
 * e.g.
 *   $$x
 *   ${$x . "_foo"}
 */
struct DynamicLocalLvalue : Lvalue {
  DynamicLocalLvalue(Compiler& c, const zend_ast* nameExpr)
    : Lvalue(c)
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

  void getB(MinstrSeq& m) override {
    getName();
    m.baseNamed(m.push());
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

  const zend_ast* nameExpr;
};

/* An lvalue corresponding to an array element that is named dynamically
 * e.g.
 *   $array[$x]
 *   $getArray()[2][getKey()]
 *
 * This composes with a base value that provides the array we index into
 */
struct DynamicDimLvalue : Lvalue {
  DynamicDimLvalue(Compiler& c,
      std::unique_ptr<BaseValue> base,
      const zend_ast* dimExpr)
    : Lvalue(c)
    , base(std::move(base))
    , dimExpr(dimExpr) {}

  inline MemberKey getKey(MinstrSeq& m, uint32_t location) {
    return CellMember{
      MemberType::Element,
      location
    };
  }

  void getC() override {
    MinstrSeq m(MOpMode::Warn);
    c.compileExpression(dimExpr, Flavor::Cell);
    auto idx = m.push();
    base->getB(m);
    m.finalCGet(getKey(m, idx));
    m.emit(c);
  }

  void getV() override {
    MinstrSeq m(MOpMode::Define);
    c.compileExpression(dimExpr, Flavor::Cell);
    auto idx = m.push();
    base->getB(m);
    m.finalVGet(getKey(m, idx));
    m.emit(c);
  }

  void getF(uint32_t slot) override {
    MinstrSeq m(slot);
    c.compileExpression(dimExpr, Flavor::Cell);
    auto idx = m.push();
    base->getB(m);
    m.finalFPass(getKey(m, idx));
    m.emit(c);
  }

  void getB(MinstrSeq& m) override {
    c.compileExpression(dimExpr, Flavor::Cell);
    auto idx = m.push();
    base->getB(m);
    m.dim(getKey(m, idx));
  }

  void assign(const zend_ast* rhs) override {
    MinstrSeq m(MOpMode::Define);
    c.compileExpression(dimExpr, Flavor::Cell);
    auto idx = m.push();
    base->getB(m);
    c.compileExpression(rhs, Flavor::Cell);
    m.pushArgument();
    m.finalSet(getKey(m, idx));
    m.emit(c);
  }

  void bind(const zend_ast* rhs) override {
    MinstrSeq m(MOpMode::Define);
    c.compileExpression(dimExpr, Flavor::Cell);
    auto idx = m.push();
    base->getB(m);
    c.compileExpression(rhs, Flavor::Ref);
    m.pushArgument();
    m.finalBind(getKey(m, idx));
    m.emit(c);
  }

  void assignOp(SetOpOp op, const zend_ast* rhs) override {
    MinstrSeq m(MOpMode::Define);
    c.compileExpression(dimExpr, Flavor::Cell);
    auto idx = m.push();
    base->getB(m);
    c.compileExpression(rhs, Flavor::Cell);
    m.pushArgument();
    m.finalSetOp(getKey(m, idx), op);
    m.emit(c);
  }

  void incDec(IncDecOp op) override {
    MinstrSeq m(MOpMode::Define);
    c.compileExpression(dimExpr, Flavor::Cell);
    auto idx = m.push();
    base->getB(m);
    m.finalIncDec(getKey(m, idx), op);
    m.emit(c);
  }

  std::unique_ptr<BaseValue> base;
  const zend_ast* dimExpr;
};

/* An lvalue corresponding to an array element that is named statically
 * e.g.
 *   $array[3]
 *   $getArray()[]
 *   $getArray()["foobar"]
 *
 * This composes with a base value that provides the array we index into
 */
struct DimLvalue : Lvalue {
  DimLvalue(Compiler& c,
      std::unique_ptr<BaseValue> base,
      const MemberKey& key)
    : Lvalue(c)
    , base(std::move(base))
    , key(key) {}

  /*
   * PHP disallows using the new-element construct in a positive position e.g.
   * echo $x[]["bar"]; this raises an error if the MemberKey is NewElement
   */
  void checkNewElemRead(const MinstrSeq& m) {
    if (boost::get<NewElem>(&key)
        && (m.isArg || m.mode != MOpMode::Define)) {
      throw LanguageException("Cannot use [] for reading");
    }
  }

  void getC() override {
    MinstrSeq m(MOpMode::Warn);
    checkNewElemRead(m);
    base->getB(m);
    m.finalCGet(key);
    m.emit(c);
  }

  void getV() override {
    MinstrSeq m(MOpMode::Define);
    checkNewElemRead(m);
    base->getB(m);
    m.finalVGet(key);
    m.emit(c);
  }

  void getF(uint32_t slot) override {
    MinstrSeq m(slot);
    checkNewElemRead(m);
    base->getB(m);
    m.finalFPass(key);
    m.emit(c);
  }

  void getB(MinstrSeq& m) override {
    checkNewElemRead(m);
    base->getB(m);
    m.dim(key);
  }

  void assign(const zend_ast* rhs) override {
    MinstrSeq m(MOpMode::Define);
    base->getB(m);
    c.compileExpression(rhs, Flavor::Cell);
    m.pushArgument();
    m.finalSet(key);
    m.emit(c);
  }

  void bind(const zend_ast* rhs) override {
    MinstrSeq m(MOpMode::Define);
    base->getB(m);
    c.compileExpression(rhs, Flavor::Ref);
    m.pushArgument();
    m.finalBind(key);
    m.emit(c);
  }

  void assignOp(SetOpOp op, const zend_ast* rhs) override {
    MinstrSeq m(MOpMode::Define);
    base->getB(m);
    c.compileExpression(rhs, Flavor::Cell);
    m.pushArgument();
    m.finalSetOp(key, op);
    m.emit(c);
  }

  void incDec(IncDecOp op) override {
    MinstrSeq m(MOpMode::Define);
    base->getB(m);
    m.finalIncDec(key, op);
    m.emit(c);
  }

  std::unique_ptr<BaseValue> base;
  MemberKey key;
};

/* This is a base value that corresponds to an arbitrary expression.
 * At runtime, getting this base pointer will fail if the expression does not
 * produce an array
 */
struct DynamicBase : BaseValue {
  DynamicBase(Compiler& c,
      const zend_ast* ast)
    : BaseValue(c)
    , ast(ast) {}

  void getB(MinstrSeq& m) override {
    if (ast->kind == ZEND_AST_CALL) {
      c.compileExpression(ast, Flavor::Return);
      m.baseRet(m.push());
    } else {
      c.compileExpression(ast, Flavor::Cell);
      m.baseCell(m.push());
    }
  }

  const zend_ast* ast;
};


std::unique_ptr<BaseValue> getBase(Compiler& c, const zend_ast* ast) {
  if (auto lv = Lvalue::getLvalue(c, ast)){
    return std::move(lv); // move necessary since we're converting to BaseValue
  } else {
    return std::make_unique<DynamicBase>(c, ast);
  }
}


} // namespace

std::unique_ptr<Lvalue> Lvalue::getLvalue(Compiler& c, const zend_ast* ast) {
  if (ast->kind == ZEND_AST_VAR) {
    auto name = ast->child[0];
    switch (name->kind) {
      case ZEND_AST_ZVAL:
        return std::make_unique<LocalLvalue>(
          c,
          zval_to_string(zend_ast_get_zval(name))
        );
      default:
        return std::make_unique<DynamicLocalLvalue>(c, name);
    }
  } else if (ast->kind == ZEND_AST_DIM) {
    auto base = ast->child[0];
    auto dim  = ast->child[1];
    if (!dim) {
      return std::make_unique<DimLvalue>(
        c,
        getBase(c, base),
        NewElem{}
      );
    } else if (dim->kind == ZEND_AST_ZVAL) {
      auto zv = zend_ast_get_zval(dim);
      switch (Z_TYPE_P(zv)) {
        case IS_LONG:
          return std::make_unique<DimLvalue>(
            c,
            getBase(c, base),
            ImmIntElem{Z_LVAL_P(zv)}
          );
        case IS_STRING:
          return std::make_unique<DimLvalue>(
            c,
            getBase(c, base),
            ImmMember{MemberType::Element, Z_STRVAL_P(zv)}
          );
        default:
          // this probably shouldn't happen, but we can just create a dynamic
          // lvalue
          break;
      }
    } else if (dim->kind == ZEND_AST_VAR) {
      auto name = dim->child[0];
      if (name->kind == ZEND_AST_ZVAL){
        auto str = zval_to_string(zend_ast_get_zval(name));
        return std::make_unique<DimLvalue>(
          c,
          getBase(c, base),
          LocalMember{MemberType::Element, Local{str}}
        );
      }
    }

    // if we get here, we can't do anything smarter:
    return std::make_unique<DynamicDimLvalue>(
      c,
      getBase(c, base),
      dim
    );
  }

  return nullptr;
}

}} // HPHP::php7
