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

#include "hphp/runtime/base/bespoke/escalation-logging.h"

#include "hphp/runtime/base/bespoke/entry-types.h"
#include "hphp/runtime/base/bespoke/logging-profile.h"
#include "hphp/runtime/vm/jit/array-layout.h"
#include "hphp/util/struct-log.h"

namespace HPHP { namespace bespoke {

//////////////////////////////////////////////////////////////////////////////

namespace {

void logEscalation(const ArrayData* ad, SrcKey sk,
                   const char* event, const char* reason) {
  auto func_str = "(unknown)";
  auto file_str = "(unknown)";
  auto line_int = 0;
  auto inst_str = std::string{"(unknown)"};
  if (sk.valid()) {
    auto const func = sk.func();
    func_str = func->fullName()->data();
    file_str = func->filename()->data();
    line_int = sk.lineNumber();
    inst_str = sk.showInst();
  }

  StructuredLogEntry entry;
  entry.setStr("source_func", func_str);
  entry.setStr("source_file", file_str);
  entry.setInt("source_line", line_int);
  entry.setStr("source_inst", inst_str);
  entry.setStr("event", event);
  entry.setStr("reason", reason);
  entry.setStr("types", EntryTypes::ForArray(ad).toString());
  entry.setStr("layout", jit::ArrayLayout::FromArray(ad).describe());

  StructuredLog::log("hhvm_bespoke_escalations", entry);
}

}

//////////////////////////////////////////////////////////////////////////////

void logGuardFailure(const ArrayData* ad, jit::ArrayLayout layout, SrcKey sk) {
  if (!StructuredLog::coinflip(RO::EvalBespokeEscalationSampleRate)) return;
  auto const reason = layout.describe();
  logEscalation(ad, sk, "GuardFailure", reason.data());
}

void logEscalateToVanilla(const BespokeArray* bad, const char* reason) {
  if (!StructuredLog::coinflip(RO::EvalBespokeEscalationSampleRate)) return;
  logEscalation(bad, getSrcKey(), "EscalateToVanilla", reason);
}

//////////////////////////////////////////////////////////////////////////////

}}
