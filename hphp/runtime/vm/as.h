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
struct MD5;

//////////////////////////////////////////////////////////////////////

/*
 * We allow callers of assemble_string to pass in a struct of callbacks which
 * can be triggered on certain events that occur during assembly. We use
 * callbacks for instance to process metadata sections like .include,
 * .constant_refs, etc. which hold information about which symbols this
 * compilation unit makes reference to but doesn't necessarily define itself.
 */
struct AsmCallbacks {
  virtual ~AsmCallbacks() {}

  virtual void onInclude(const std::string&) {}
  virtual void onConstantRef(const std::string&) {}
  virtual void onFunctionRef(const std::string&) {}
  virtual void onClassRef(const std::string&) {}
};

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
  const MD5&,
  bool swallowErrors = true,
  AsmCallbacks* callbacks = nullptr
);

enum class AsmResult {
  NoResult,
  ValuePushed,
  Unreachable
};

AsmResult assemble_expression(UnitEmitter&, FuncEmitter*, int,
                              const std::string&);

//////////////////////////////////////////////////////////////////////

}

#endif
