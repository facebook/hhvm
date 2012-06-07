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

#ifndef incl_INSTRUMENTATION_HOOK_H_
#define incl_INSTRUMENTATION_HOOK_H_

#include <runtime/vm/instrumentation.h>
#include <runtime/base/execution_context.h>

namespace HPHP {
namespace VM {

static inline void instHookInt64Impl(InjectionTableInt64* table, int64 val) {
  if (!table) return;
  InjectionTableInt64::const_iterator it = table->find(val);
  if (LIKELY(it == table->end())) return;
  it->second->execute();
}

static inline void instHookInt64(int type, int64 val) {
  ASSERT(type < InstHookTypeInt64Count);
  InjectionTableInt64* injTable = g_vmContext->m_injTables ?
    g_vmContext->m_injTables->getInt64Table(type) : NULL;
  instHookInt64Impl(injTable, val);
}

static inline void instHookSD(int type, const StringData* sd) {
  ASSERT(type < InstHookTypeSDCount);
  InjectionTableSD* injTable = g_vmContext->m_injTables ?
    g_vmContext->m_injTables->getSDTable(type) : NULL;
  if (LIKELY(!injTable)) return;
  InjectionTableSD::const_iterator it = injTable->find(sd);
  if (LIKELY(it == injTable->end())) return;
  it->second->execute();
}

static inline void instHookStr(int type, const char* str) {
  StringData sd(str, AttachLiteral);
  instHookSD(type, &sd);
}

#define INST_HOOK_PC(table, pc) instHookInt64Impl(table, (int64)pc)

#define INST_HOOK_EVT_STR(str)  instHookStr(InstHookTypeCustomEvt, str)
#define INST_HOOK_EVT_SD(sd)    instHookSD(InstHookTypeCustomEvt, sd)

#define INST_HOOK_FENTRY(sd)    instHookSD(InstHookTypeFuncEntry, sd)

} }    // HPHP::VM

#endif /* incl_INSTRUMENTATION_HOOK_H_ */
