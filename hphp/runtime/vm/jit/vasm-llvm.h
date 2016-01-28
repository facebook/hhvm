/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_JIT_VASM_LLVM_H_
#define incl_HPHP_JIT_VASM_LLVM_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"

#include <folly/Format.h>

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

struct Vtext;
struct Vunit;

///////////////////////////////////////////////////////////////////////////////

/*
 * Thrown when the LLVM backend encounters something it doesn't support.
 */
struct FailedLLVMCodeGen : public std::runtime_error {
  template<typename... Args>
  explicit FailedLLVMCodeGen(Args&&... args)
    : std::runtime_error(folly::sformat(std::forward<Args>(args)...))
  {}
};

/*
 * Thrown when the llvm_compare trace module is active, to allow comparing LLVM
 * and vasm output.
 */
struct CompareLLVMCodeGen : FailedLLVMCodeGen {
  explicit CompareLLVMCodeGen(jit::vector<std::string>&& disasm,
                              std::string&& llvm,
                              size_t main_size)
    : FailedLLVMCodeGen("Discarding LLVM code for comparison")
    , disasm(std::move(disasm))
    , llvm(std::move(llvm))
    , main_size(main_size)
  {}

  jit::vector<std::string> disasm;
  std::string llvm;
  size_t main_size;
};

/*
 * Emit machine code for unit using the LLVM backend.
 *
 * Throws FailedLLVMCodeGen on failure. Any code/data emitted to the given
 * areas is *not* cleaned up on failure; the caller must decide how to clean
 * up.
 */
void genCodeLLVM(const Vunit& unit, Vtext& text);

///////////////////////////////////////////////////////////////////////////////
}}

#endif
