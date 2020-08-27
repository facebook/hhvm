/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_HHVM_AS_H_
#define incl_HPHP_HHVM_AS_H_

#include <memory>
#include <string>

namespace HPHP {

struct UnitEmitter;
struct FuncEmitter;
struct SHA1;

namespace Native {
struct FuncTable;
}

//////////////////////////////////////////////////////////////////////

/*
 * Assemble the contents of `filename' and return a UnitEmitter.
 *
 * If swallowErrors is true then emit a fataling unit for any assembler errors.
 *
 * Minimal documentation is available in as.cpp.
 */
std::unique_ptr<UnitEmitter> assemble_string(
  const char* code,
  int codeLen,
  const char* filename,
  const SHA1&,
  const Native::FuncTable&,
  bool swallowErrors = true,
  bool wantsSymbolRefs = false
);

enum class AsmResult {
  NoResult,
  ValuePushed,
  Unreachable
};

struct AssemblerFatal : std::runtime_error {
  explicit AssemblerFatal(const std::string& msg) : std::runtime_error(msg) {}
};

struct AssemblerError : std::runtime_error {
  explicit AssemblerError(const std::string& msg) : std::runtime_error(msg) {}
  AssemblerError(int where, const std::string& what);
};

struct AssemblerUnserializationError : AssemblerError {
  using AssemblerError::AssemblerError;
};

//////////////////////////////////////////////////////////////////////

}

#endif
