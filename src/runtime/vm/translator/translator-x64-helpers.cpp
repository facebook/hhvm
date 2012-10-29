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
#include <runtime/vm/translator/translator-deps.h>
#include <runtime/vm/translator/translator-inline.h>
#include <runtime/vm/translator/translator-x64.h>
#include <runtime/vm/translator/targetcache.h>
#include <runtime/eval/runtime/file_repository.h>
#include <runtime/vm/event_hook.h>
#include <runtime/vm/stats.h>

namespace HPHP {
namespace VM {
namespace Transl {

inline bool
checkEval(HPHP::Eval::PhpFile* efile) {
  return !TargetCache::testAndSetBit(efile->getId());
}

/*
 * reserve rbx within this translation unit
 *
 * Note that for rbp we use a local register asm
 * variable. But for rbx, we need to update it, and
 * local register asm vars are saved and restored just
 * like any other local. So I put this function in its own
 * file to avoid impacting the rest of translator-x64.cpp
 */
register Cell* sp asm("rbx");

void TranslatorX64::reqLitHelper(const ReqLitStaticArgs* args) {
  register ActRec* rbp asm("rbp");

  HPHP::Eval::PhpFile* efile = args->m_efile;
  if (!checkEval(efile)) {
    Stats::inc(Stats::PseudoMain_Guarded);
    sp->m_type = KindOfBoolean;
    sp->m_data.num = 1;
    return;
  }

  ActRec* fp = (ActRec*)rbp->m_savedRbp;

  VMExecutionContext *ec = g_vmContext;
  ec->m_fp = fp;
  ec->m_stack.top() = sp + 1;
  PC pc = curUnit()->at(args->m_pcOff);

  tl_regState = REGSTATE_CLEAN;
  Unit *unit = efile->unit();
  bool runPseudoMain = ec->evalUnit(unit, args->m_local, pc,
                                    EventHook::PseudoMain);
  tl_regState = REGSTATE_DIRTY;
  if (!runPseudoMain) return;

  ec->m_fp->m_savedRip = rbp->m_savedRip;
  // smash our return and rbp chain
  rbp->m_savedRip = (uint64_t)args->m_pseudoMain;
  sp = ec->m_stack.top();
  rbp->m_savedRbp = (uint64_t)ec->m_fp;
}

asm (
  ".byte 0\n"
  ".align 16\n"
  ".globl __funcBodyHelperThunk\n"
"__funcBodyHelperThunk:\n"
#ifdef HHVM
  "mov %rbp, %rdi\n"
  "call funcBodyHelper\n"
  "jmp *%rax\n"
#endif
  "ud2\n"
);
TCA funcBodyHelper(ActRec* fp) {
  g_vmContext->m_fp = fp;
  g_vmContext->m_stack.top() = sp;
  g_vmContext->m_pc = fp->m_func->unit()->at(fp->m_func->base());
  tl_regState = REGSTATE_CLEAN;
  Func* func = const_cast<Func*>(fp->m_func);
  SrcKey sk(func, func->base());
  TCA tca = tx64->getCallArrayProlog(func);
  if (tca) {
    func->setFuncBody(tca);
  } else {
    tca = Translator::Get()->getResumeHelper();
  }
  tl_regState = REGSTATE_DIRTY;
  return tca;
}

void TranslatorX64::fCallArrayHelper(const FCallArrayArgs* args) {
  register ActRec* rbp asm("rbp");
  ActRec* fp = (ActRec*)rbp->m_savedRbp;

  VMExecutionContext *ec = g_vmContext;
  ec->m_fp = fp;
  ec->m_stack.top() = sp;
  ec->m_pc = curUnit()->at(args->m_pcOff);
  PC pc = curUnit()->at(args->m_pcNext);

  tl_regState = REGSTATE_CLEAN;
  bool runFunc = ec->doFCallArray(pc);
  sp = ec->m_stack.top();
  tl_regState = REGSTATE_DIRTY;
  if (!runFunc) return;

  ec->m_fp->m_savedRip = rbp->m_savedRip;
  // smash our return and rbp chain
  rbp->m_savedRip = (uint64_t)ec->m_fp->m_func->getFuncBody();
  rbp->m_savedRbp = (uint64_t)ec->m_fp;
}

} } } // HPHP::VM::Transl
