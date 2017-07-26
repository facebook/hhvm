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

#include "hphp/php7/zend/zend.h"
#include "hphp/php7/ast_info.h"
#include "hphp/php7/bytecode.h"
#include "hphp/php7/unit.h"

#include <folly/ScopeGuard.h>

#include <exception>
#include <stack>
#include <string>

namespace HPHP { namespace php7 {

struct CompilerException : public std::logic_error {
  explicit CompilerException(const std::string& what)
    : std::logic_error(what) {}
};

struct LanguageException : CompilerException {
  explicit LanguageException(const std::string& what)
    : CompilerException(what) {}
};

enum Flavor {
  Drop,
  Cell,
  Ref,
  Return,
  FuncParam,
};

/*
 * Since we need the argument number to push a F flavored value, this lets us
 * track expected flavor + argument number
 *
 * The argument slot is nonzero only when flavor is FuncParam
 */
struct Destination {
 private:
  Destination(Flavor flavor, uint32_t slot)
    : flavor(flavor)
    , slot(slot) {}

 public:
  /* implicit */ Destination(Flavor flavor)
    : flavor(flavor)
    , slot(0) {
    assert(flavor != Flavor::FuncParam);
  }

  static Destination Stack(Flavor flavor) {
    return flavor;
  }

  static Destination Param(uint32_t slot) {
    return {Flavor::FuncParam, slot};
  }

  const Flavor flavor;
  const uint32_t slot;
};


struct Compiler {
  explicit Compiler(const std::string& filename);

  static std::unique_ptr<Unit> compile(const std::string& filename,
                                       const zend_ast* ast) {
    Compiler compiler(filename);
    compiler.compileProgram(ast);
    return std::move(compiler.unit);
  }

 private:
  void compileProgram(const zend_ast* ast);
  void compileFunction(const zend_ast* ast);
  void compileStatement(const zend_ast* ast);
  void compileExpression(const zend_ast* ast, Destination destination);

  void compileZvalLiteral(const zval* ast);
  void compileConstant(const zend_ast* ast);
  void compileVar(const zend_ast* ast, Destination destination);
  void compileAssignment(const zend_ast* ast);
  void compileBind(const zend_ast* ast);
  void compileAssignOp(const zend_ast* ast);
  void compileCall(const zend_ast* ast);
  void compileArray(const zend_ast* ast);

  void compileGlobalDeclaration(const zend_ast* ast);
  void compileIf(const zend_ast* ast);
  void compileWhile(const zend_ast* cond, const zend_ast* body, bool bodyFirst);
  void compileFor(const zend_ast* ast);

  Bytecode opForBinaryOp(const zend_ast* op);
  IncDecOp getIncDecOpForNode(zend_ast_kind kind);
  SetOpOp getSetOpOp(zend_ast_attr attr);

  void compileUnaryOp(const zend_ast* op);
  void compileIncDec(const zend_ast* op);

  [[noreturn]]
  void panic(const std::string& msg);

  void fixFlavor(Destination dest, Flavor actual);

  template<class Branch>
  void branchTo(Block* target) {
    auto continuation = activeFunction->allocateBlock();

    activeBlock->exit(Branch{target});
    activeBlock->exit(bc::Jmp{continuation});

    activeBlock = continuation;
  }

  template<class Function>
  void withBlock(Block* blk, Function&& f) {
    std::swap(blk, activeBlock);
    SCOPE_EXIT { std::swap(blk, activeBlock); };
    f();
  }

  struct Lvalue {
    virtual ~Lvalue() = default;

    virtual void getC() = 0;
    virtual void getV() = 0;
    virtual void getF(uint32_t slot) = 0;
    virtual void assign(const zend_ast* rhs) = 0;
    virtual void bind(const zend_ast* rhs) = 0;
    virtual void assignOp(SetOpOp op, const zend_ast* rhs) = 0;
    virtual void incDec(IncDecOp op) = 0;
  };

  struct LocalLvalue;
  struct DynamicLocalLvalue;

  std::unique_ptr<Lvalue> getLvalue(const zend_ast* ast);


  std::unique_ptr<Unit> unit;

  Function* activeFunction;
  Block* activeBlock;

  // relevant block addresses for a loop
  struct Loop {
    Block* continuation; // label immediately after loop
    Block* test;         // label at loop test (for continue)
  };

  std::stack<Loop> activeLoops;

  Loop& currentLoop(const std::string& forWhat) {
    if (activeLoops.empty()) {
      throw LanguageException(
          folly::sformat("'{}' not in the 'loop' or 'switch' context",
                         forWhat));
    }
    return activeLoops.top();
  }

};

}} // HPHP::php7


#endif // incl_HPHP_PHP7_COMPILER_H_
