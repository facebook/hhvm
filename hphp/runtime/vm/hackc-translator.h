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

#include "hphp/hack/src/hackc/ffi_bridge/compiler_ffi.rs.h"
#include "hphp/hack/src/hackc/hhbc-unit.h"
#include "hphp/runtime/vm/native-func-table.h"
#include "hphp/runtime/vm/unit-emitter.h"

namespace HPHP {

enum class CodeSource;

inline const hackc::hhbc::Unit* hackCUnitRaw(const rust::Box<hackc::UnitWrapper>& unit) {
  return (const hackc::hhbc::Unit*)(&(*unit));
}

std::unique_ptr<UnitEmitter> unitEmitterFromHackCUnit(
  const hackc::hhbc::Unit& unit,
  const char* filename,
	const SHA1& sha1,
  const SHA1& bcSha1,
  const Extension* extension,
  bool swallowErrors,
  const PackageInfo&
);

}
