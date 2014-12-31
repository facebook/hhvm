/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/vasm-x64.h"

#include <folly/Format.h>

namespace HPHP { namespace jit {

/*
 * Thrown when the LLVM backend encounters something it doesn't support.
 */
struct FailedLLVMCodeGen : public std::runtime_error {
 public:
  template<typename... Args>
  explicit FailedLLVMCodeGen(Args&&... args)
    : std::runtime_error(folly::sformat(std::forward<Args>(args)...))
  {}
};

/*
 * Emit machine code for unit using the LLVM backend.
 *
 * Throws FailedLLVMCodeGen on failure.
 */
void genCodeLLVM(const Vunit& unit, Vasm::AreaList& areas,
                 const jit::vector<Vlabel>& labels);

} }

#endif
