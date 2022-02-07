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

#pragma once

#include "hphp/hack/src/hackc/ffi_bridge/compiler_ffi.rs"
#include "hphp/hack/src/hackc/hhbc-ast.h"
#include "hphp/runtime/vm/native-func-table.h"
#include "hphp/runtime/vm/unit-emitter.h"

namespace HPHP {

struct TranslationFatal : std::runtime_error {
  explicit TranslationFatal(const std::string& msg) : std::runtime_error(msg) {}
};

inline const hackc::hhbc::HhasProgram* hhasProgramRaw(const ::rust::Box<HhasProgramWrapper>& program) {
  return (const hackc::hhbc::HhasProgram*)(&(*program));
}

std::unique_ptr<UnitEmitter> unitEmitterFromHhasProgram(
  const hackc::hhbc::HhasProgram& prog,
  const char* filename,
	const SHA1& sha1,
  const Native::FuncTable& nativeFuncs,
  const std::string& hhasString
);

}
