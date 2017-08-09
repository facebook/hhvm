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

  CFG finalQuery(const MemberKey& key, QueryMOp op) {
    instrs.push_back(QueryM{ keysOnStack, op, key});
    return emit();
  }

  CFG finalCGet(const MemberKey& key) {
    return finalQuery(key, QueryMOp::CGet);
  }

  CFG finalFPass(const MemberKey& key) {
    assert(isArg);
    instrs.push_back(FPassM{ slot, keysOnStack, key});
    return emit();
  }

  CFG finalIsset(const MemberKey& key) {
    return finalQuery(key, QueryMOp::Isset);
  }

  CFG finalEmpty(const MemberKey& key) {
    return finalQuery(key, QueryMOp::Empty);
  }

  CFG finalVGet(const MemberKey& key) {
    instrs.push_back(VGetM{keysOnStack, key});
    return emit();
  }

  CFG finalSet(const MemberKey& key) {
    instrs.push_back(SetM{keysOnStack, key});
    return emit();
  }

  CFG finalSetOp(const MemberKey& key, SetOpOp op) {
    instrs.push_back(SetOpM{keysOnStack, op, key});
    return emit();
  }

  CFG finalBind(const MemberKey& key) {
    instrs.push_back(BindM{keysOnStack, key});
    return emit();
  }

  CFG finalIncDec(const MemberKey& key, IncDecOp op) {
    instrs.push_back(IncDecM{keysOnStack, op, key});
    return emit();
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

  CFG emit() {
    CFG cfg;
    for (auto& bc : instrs) {
      bc.visit(MemberKeyVisitor(stackDepth));
      cfg.then(std::move(bc));
    }
    return cfg;
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
  explicit LocalLvalue(std::string name)
    : name(std::move(name)) {}

  CFG getC() override {
    return { CGetL{name} };
  }

  CFG getV() override {
    return { VGetL{name} };
  }

  CFG getF(uint32_t slot) override {
    return { FPassL{slot, name} };
  }

  CFG getB(MinstrSeq& m) override {
    m.baseLocal(Local{name});
    return {};
  }

  CFG assign(const zend_ast* rhs) override {
    return compileExpression(rhs, Flavor::Cell)
      .then(SetL{name});
  }

  CFG bind(const zend_ast* rhs) override {
    return compileExpression(rhs, Flavor::Ref)
      .then(BindL{name});
  }

  CFG assignOp(SetOpOp op, const zend_ast* rhs) override {
    return compileExpression(rhs, Flavor::Cell)
      .then(SetOpL{name, op});
  }

  CFG incDec(IncDecOp op) override {
    return { IncDecL{name, op} };
  }

  std::string name;
};

/* An lvalue corresponding to a local that is named dynamically
 * e.g.
 *   $$x
 *   ${$x . "_foo"}
 */
struct DynamicLocalLvalue : Lvalue {
  explicit DynamicLocalLvalue(const zend_ast* nameExpr)
    : nameExpr(nameExpr) {}

  CFG getName() {
    return compileExpression(nameExpr, Flavor::Cell);
  }

  CFG getC() override {
    return getName()
      .then(CGetN{});
  }

  CFG getV() override {
    return getName()
      .then(VGetN{});
  }

  CFG getF(uint32_t slot) override {
    return getName()
      .then(FPassN{slot});
  }

  CFG getB(MinstrSeq& m) override {
    m.baseNamed(m.push());
    return getName();
  }

  CFG assign(const zend_ast* rhs) override {
    return getName()
      .then(compileExpression(rhs, Flavor::Cell))
      .then(SetN{});
  }

  CFG bind(const zend_ast* rhs) override {
    return getName()
      .then(compileExpression(rhs, Flavor::Ref))
      .then(BindN{});
  }

  CFG assignOp(SetOpOp op, const zend_ast* rhs) override {
    return getName()
      .then(compileExpression(rhs, Flavor::Cell))
      .then(SetOpN{op});
  }

  CFG incDec(IncDecOp op) override {
    return getName()
      .then(IncDecN{op});
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
  DynamicDimLvalue(std::unique_ptr<BaseValue> base,
      const zend_ast* dimExpr)
    : base(std::move(base))
    , dimExpr(dimExpr) {}

  inline MemberKey getKey(MinstrSeq& m, uint32_t location) {
    return CellMember{
      MemberType::Element,
      location
    };
  }

  CFG getC() override {
    MinstrSeq m(MOpMode::Warn);
    auto idx = m.push();
    auto setup = compileExpression(dimExpr, Flavor::Cell)
      .then(base->getB(m));
    return setup.then(m.finalCGet(getKey(m, idx)));
  }

  CFG getV() override {
    MinstrSeq m(MOpMode::Define);
    auto idx = m.push();
    auto setup = compileExpression(dimExpr, Flavor::Cell)
      .then(base->getB(m));
    return setup.then(m.finalVGet(getKey(m, idx)));
  }

  CFG getF(uint32_t slot) override {
    MinstrSeq m(slot);
    auto idx = m.push();
    auto setup = compileExpression(dimExpr, Flavor::Cell)
      .then(base->getB(m));
    return setup.then(m.finalFPass(getKey(m, idx)));
  }

  CFG getB(MinstrSeq& m) override {
    auto cfg = compileExpression(dimExpr, Flavor::Cell)
      .then(base->getB(m));
    auto idx = m.push();
    m.dim(getKey(m, idx));
    return cfg;
  }

  CFG assign(const zend_ast* rhs) override {
    MinstrSeq m(MOpMode::Define);
    auto idx = m.push();
    auto setup = compileExpression(dimExpr, Flavor::Cell)
      .then(base->getB(m));
    m.pushArgument();
    return setup
      .then(compileExpression(rhs, Flavor::Cell))
      .then(m.finalSet(getKey(m, idx)));
  }

  CFG bind(const zend_ast* rhs) override {
    MinstrSeq m(MOpMode::Define);
    auto idx = m.push();
    auto setup = compileExpression(dimExpr, Flavor::Cell)
      .then(base->getB(m));
    m.pushArgument();
    return setup
      .then(compileExpression(rhs, Flavor::Ref))
      .then(m.finalBind(getKey(m, idx)));
  }

  CFG assignOp(SetOpOp op, const zend_ast* rhs) override {
    MinstrSeq m(MOpMode::Define);
    auto idx = m.push();
    auto setup = compileExpression(dimExpr, Flavor::Cell)
      .then(base->getB(m));
    m.pushArgument();
    return setup
      .then(compileExpression(rhs, Flavor::Cell))
      .then(m.finalSetOp(getKey(m, idx), op));
  }

  CFG incDec(IncDecOp op) override {
    MinstrSeq m(MOpMode::Define);
    auto idx = m.push();
    auto setup = compileExpression(dimExpr, Flavor::Cell)
      .then(base->getB(m));
    return setup.then(m.finalIncDec(getKey(m, idx), op));
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
  DimLvalue(std::unique_ptr<BaseValue> base,
      const MemberKey& key)
    : base(std::move(base))
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

  CFG getC() override {
    MinstrSeq m(MOpMode::Warn);
    checkNewElemRead(m);
    auto setup = base->getB(m);
    return setup.then(m.finalCGet(key));
  }

  CFG getV() override {
    MinstrSeq m(MOpMode::Define);
    checkNewElemRead(m);
    auto setup = base->getB(m);
    return setup.then(m.finalVGet(key));
  }

  CFG getF(uint32_t slot) override {
    MinstrSeq m(slot);
    checkNewElemRead(m);
    auto setup = base->getB(m);
    return setup.then(m.finalFPass(key));
  }

  CFG getB(MinstrSeq& m) override {
    checkNewElemRead(m);
    auto setup = base->getB(m);
    m.dim(key);
    return setup;
  }

  CFG assign(const zend_ast* rhs) override {
    MinstrSeq m(MOpMode::Define);
    auto setup = base->getB(m);

    m.pushArgument();
    return setup
      .then(compileExpression(rhs, Flavor::Cell))
      .then(m.finalSet(key));
  }

  CFG bind(const zend_ast* rhs) override {
    MinstrSeq m(MOpMode::Define);
    auto setup = base->getB(m);
    m.pushArgument();
    return setup
      .then(compileExpression(rhs, Flavor::Ref))
      .then(m.finalBind(key));
  }

  CFG assignOp(SetOpOp op, const zend_ast* rhs) override {
    MinstrSeq m(MOpMode::Define);
    auto setup = base->getB(m);
    m.pushArgument();
    return setup
      .then(compileExpression(rhs, Flavor::Cell))
      .then(m.finalSetOp(key, op));
  }

  CFG incDec(IncDecOp op) override {
    MinstrSeq m(MOpMode::Define);
    auto setup = base->getB(m);
    return setup.then(m.finalIncDec(key, op));
  }

  std::unique_ptr<BaseValue> base;
  MemberKey key;
};

/* This is a base value that corresponds to an arbitrary expression.
 * At runtime, getting this base pointer will fail if the expression does not
 * produce an array
 */
struct DynamicBase : BaseValue {
  explicit DynamicBase(const zend_ast* ast)
    : ast(ast) {}

  CFG getB(MinstrSeq& m) override {
    if (ast->kind == ZEND_AST_CALL) {
      m.baseRet(m.push());
      return compileExpression(ast, Flavor::Return);
    } else {
      m.baseCell(m.push());
      return compileExpression(ast, Flavor::Cell);
    }
  }

  const zend_ast* ast;
};

std::unique_ptr<BaseValue> getBase(const zend_ast* ast) {
  if (auto lv = Lvalue::getLvalue(ast)){
    return std::move(lv); // move necessary since we're converting to BaseValue
  } else {
    return std::make_unique<DynamicBase>(ast);
  }
}

} // namespace

std::unique_ptr<Lvalue> Lvalue::getLvalue(const zend_ast* ast) {
  if (ast->kind == ZEND_AST_VAR) {
    auto name = ast->child[0];
    switch (name->kind) {
      case ZEND_AST_ZVAL:
        return std::make_unique<LocalLvalue>(
          zval_to_string(zend_ast_get_zval(name))
        );
      default:
        return std::make_unique<DynamicLocalLvalue>(name);
    }
  } else if (ast->kind == ZEND_AST_DIM) {
    auto base = ast->child[0];
    auto dim  = ast->child[1];
    if (!dim) {
      return std::make_unique<DimLvalue>(
        getBase(base),
        NewElem{}
      );
    } else if (dim->kind == ZEND_AST_ZVAL) {
      auto zv = zend_ast_get_zval(dim);
      switch (Z_TYPE_P(zv)) {
        case IS_LONG:
          return std::make_unique<DimLvalue>(
            getBase(base),
            ImmIntElem{Z_LVAL_P(zv)}
          );
        case IS_STRING:
          return std::make_unique<DimLvalue>(
            getBase(base),
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
          getBase(base),
          LocalMember{MemberType::Element, Local{str}}
        );
      }
    }

    // if we get here, we can't do anything smarter:
    return std::make_unique<DynamicDimLvalue>(
      getBase(base),
      dim
    );
  }

  return nullptr;
}

}} // HPHP::php7
