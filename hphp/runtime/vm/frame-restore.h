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

#ifndef incl_HPHP_VM_FRAME_RESTORE_H_
#define incl_HPHP_VM_FRAME_RESTORE_H_

#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/vm/preclass.h"
#include "hphp/runtime/vm/record.h"
#include "hphp/runtime/vm/type-alias.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct FrameRestore : private VMRegAnchor {
  explicit FrameRestore(const PreClass* preClass) :
    FrameRestore(VMParserFrame { preClass->unit()->filepath(), preClass->line1() }) {}

  explicit FrameRestore(const PreRecordDesc* pr) :
    FrameRestore(VMParserFrame { pr->unit()->filepath(), pr->line1() }) {}

  explicit FrameRestore(const PreTypeAlias* ta) :
    FrameRestore(VMParserFrame { ta->unit->filepath(), ta->line1 }) {}

  explicit NEVER_INLINE FrameRestore(const VMParserFrame& parserframe) :
    m_parserframe(parserframe) {
    m_oldParserframe = BacktraceArgs::setGlobalParserFrame(&m_parserframe);
  }

  ~FrameRestore() {
    BacktraceArgs::setGlobalParserFrame(m_oldParserframe);
  }

 private:
  VMParserFrame m_parserframe;
  VMParserFrame* m_oldParserframe;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_VM_FRAME_RESTORE_H_
