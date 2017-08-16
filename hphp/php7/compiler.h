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

#include "hphp/php7/ast_info.h"
#include "hphp/php7/bytecode.h"
#include "hphp/php7/cfg.h"
#include "hphp/php7/unit.h"
#include "hphp/php7/lvalue.h"
#include "hphp/php7/zend/zend.h"

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

void compileProgram(Unit* unit, const zend_ast* ast);
void compileFunction(Unit* unit, const zend_ast* ast);
CFG compileClass(Unit* unit, const zend_ast* ast);
CFG compileStatement(Function* func, const zend_ast* ast);
CFG compileExpression(const zend_ast* ast, Destination destination);

inline std::unique_ptr<Unit> compile(const std::string& filename,
                                     const zend_ast* ast) {
  auto unit = std::make_unique<Unit>();
  unit->name = filename;
  compileProgram(unit.get(), ast);
  return unit;
}

[[noreturn]]
void panic(const std::string& msg);


}} // HPHP::php7


#endif // incl_HPHP_PHP7_COMPILER_H_
