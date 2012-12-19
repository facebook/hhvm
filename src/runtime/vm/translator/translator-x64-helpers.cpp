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

/*
 * fcallHelperThunk--
 *
 *   Func's prologue entries initially point here. Vector into fcallHelper,
 *   where we can translate a new prologue.
 */

static TCA callAndResume(ActRec *ar) {
  VMRegAnchor _(ar, true);
  g_vmContext->doFCall<true>(ar, g_vmContext->m_pc);
  return Translator::Get()->getResumeHelperRet();
}

static_assert(rStashedAR == reg::r15,
  "__fcallHelperThunk needs to be modified for ABI changes");
asm(
  ".byte 0\n"
  ".align 16\n"
  ".globl __fcallHelperThunk\n"
"__fcallHelperThunk:\n"
  // This assembly code isn't PIC-friendly. HHVM doesn't care, but HPHP
  // still builds a .so.
#ifdef HHVM
  // fcallHelper may call doFCall. doFCall changes the return ip
  // pointed to by r15 so that it points to TranslatorX64::m_retHelper,
  // which does a REQ_POST_INTERP_RET service request. So we need to
  // to pop the return address into r15 + m_savedRip before calling
  // fcallHelper, and then push it back from r15 + m_savedRip after
  // fcallHelper returns in case it has changed it.
  "pop 0x8(%r15)\n"
  "mov %r15, %rdi\n"
  "call fcallHelper\n"
  "push 0x8(%r15)\n"
  "jmp *%rax\n"
#endif
  "ud2\n"
);

extern "C"
TCA fcallHelper(ActRec* ar) {
  try {
    TCA tca =
      Translator::Get()->funcPrologue((Func*)ar->m_func, ar->numArgs());
    if (tca) {
      return tca;
    }
    return callAndResume(ar);
  } catch (...) {
    /*
      The return address is set to __fcallHelperThunk,
      which has no unwind information. Its "logically"
      part of the tc, but the c++ unwinder wont know
      that. So point our return address at the called
      function's return address (which will be in the
      tc).
      Note that the registers really are clean - we
      just came from callAndResume which cleaned
      them for us - so we just have to tell the unwinder
      that.
    */
    register ActRec* rbp asm("rbp");
    tl_regState = REGSTATE_CLEAN;
    rbp->m_savedRip = ar->m_savedRip;
    throw;
  }
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

asm (
  ".byte 0\n"
  ".align 16\n"
  ".globl __contEnterHelperThunk\n"
"__contEnterHelperThunk:\n"
#ifdef HHVM
  // The generator body's AR is in rStashedAR. rVmFp still points to the frame
  // above the generator. The prologue is responsible for setting rVmFp. Even
  // if we can't get a prologue, funcBodyHelper syncs the new FP, and the
  // "resume helper" sets the hardward FP from that.
  "mov %r15, %rdi\n"
  "call funcBodyHelper\n"
  "jmp *%rax\n"
#endif
  "ud2\n"
);

/*
 * Two different "function"-entry paths come through here: entering functions
 * via FCallArray, and entering generator bodies. The common element is that
 * neither requires different entry points for different callsite arities.
 */
TCA funcBodyHelper(ActRec* fp) {
  g_vmContext->m_fp = fp;
  g_vmContext->m_stack.top() = sp;
  int nargs = fp->numArgs();
  int nparams = fp->m_func->numParams();
  Offset firstDVInitializer = InvalidAbsoluteOffset;
  if (nargs < nparams) {
    const Func::ParamInfoVec& paramInfo = fp->m_func->params();
    for (int i = nargs; i < nparams; ++i) {
      Offset dvInitializer = paramInfo[i].funcletOff();
      if (dvInitializer != InvalidAbsoluteOffset) {
        firstDVInitializer = dvInitializer;
        break;
      }
    }
  }
  if (firstDVInitializer != InvalidAbsoluteOffset) {
    g_vmContext->m_pc = fp->m_func->unit()->entry() + firstDVInitializer;
  } else {
    g_vmContext->m_pc = fp->m_func->getEntry();
  }
  tl_regState = REGSTATE_CLEAN;
  Func* func = const_cast<Func*>(fp->m_func);
  SrcKey sk(func, func->base());

  TCA tca;
  if (func->isGenerator()) {
    ASSERT(nargs == 1);
    tca = tx64->funcPrologue(func, nargs);
  } else {
    tca = tx64->getCallArrayProlog(func);
  }

  if (tca) {
    func->setFuncBody(tca);
  } else {
    if (func->isGenerator()) {
      tca = Translator::Get()->getResumeHelperRet();
    } else {
      tca = Translator::Get()->getResumeHelper();
    }
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
