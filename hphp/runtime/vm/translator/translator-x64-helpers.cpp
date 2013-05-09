/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include <runtime/base/stats.h>

namespace HPHP {
namespace VM {
namespace Transl {

inline bool
checkEval(HPHP::Eval::PhpFile* efile) {
  return !TargetCache::testAndSetBit(efile->getId());
}

/*
 * reserve the VM stack pointer within this translation unit
 *
 * Note that for rbp we use a local register asm
 * variable. But for rbx, we need to update it, and
 * local register asm vars are saved and restored just
 * like any other local. So I put this function in its own
 * file to avoid impacting the rest of translator-x64.cpp
 */
#ifdef __x86_64__
register Cell* sp asm("rbx");
#else
// TODO(#2056140): we have to decide regalloc conventions for ARM
Cell* sp;
#endif

void TranslatorX64::reqLitHelper(const ReqLitStaticArgs* args) {
  DECLARE_FRAME_POINTER(framePtr);

  HPHP::Eval::PhpFile* efile = args->m_efile;
  if (!checkEval(efile)) {
    Stats::inc(Stats::PseudoMain_Guarded);
    sp->m_type = KindOfBoolean;
    sp->m_data.num = 1;
    return;
  }

  ActRec* fp = (ActRec*)framePtr->m_savedRbp;

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

  ec->m_fp->m_savedRip = framePtr->m_savedRip;
  // smash our return and frame pointer chain
  framePtr->m_savedRip = (uint64_t)args->m_pseudoMain;
  sp = ec->m_stack.top();
  framePtr->m_savedRbp = (uint64_t)ec->m_fp;
}

static void setupAfterProlog(ActRec* fp) {
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
}

/*
 * fcallHelperThunk--
 *
 *   Func's prologue entries initially point here. Vector into fcallHelper,
 *   where we can translate a new prologue.
 */

static TCA callAndResume(ActRec *ar) {
  if (!ar->m_func->isClonedClosure()) {
    /*
     * If the func is a cloned closure, then the original
     * closure has already run the prolog, and the prologs
     * array is just being used as entry points for the
     * dv funclets. Dont run the prolog again.
     */
    VMRegAnchor _(ar);
    uint64_t rip = ar->m_savedRip;
    g_vmContext->doFCall<true>(ar, g_vmContext->m_pc);
    ar->m_savedRip = rip;
    return Translator::Get()->getResumeHelperRet();
  }
  setupAfterProlog(ar);
  return Translator::Get()->getResumeHelper();
}

static_assert(rStashedAR == reg::r15,
  "__fcallHelperThunk needs to be modified for ABI changes");
asm(
  ".byte 0\n"
  ".align 16\n"
  ".globl __fcallHelperThunk\n"
"__fcallHelperThunk:\n"
#if defined(__x86_64__)
  // fcallHelper is used for prologs, and (in the case of
  // closures) for dispatch to the function body. In the first
  // case, there's a call, in the second, there's a jmp.
  // We can differentiate by comparing r15 and rVmFp
  "mov %r15, %rdi\n"
  "cmp %r15, %rbp\n"
  "jne 1f\n"
  "call fcallHelper\n"
  "jmp *%rax\n"
  // fcallHelper may call doFCall. doFCall changes the return ip
  // pointed to by r15 so that it points to TranslatorX64::m_retHelper,
  // which does a REQ_POST_INTERP_RET service request. So we need to
  // to pop the return address into r15 + m_savedRip before calling
  // fcallHelper, and then push it back from r15 + m_savedRip after
  // fcallHelper returns in case it has changed it.
  "1: pop 0x8(%r15)\n"
  "call fcallHelper\n"
  "push 0x8(%r15)\n"
  "jmp *%rax\n"
  "ud2\n"
#elif defined(__AARCH64EL__)
  "brk 0\n"
#else
# error You sure have your work cut out for you
#endif
);

extern "C"
TCA fcallHelper(ActRec* ar) {
  try {
    TCA tca =
      Translator::Get()->funcPrologue((Func*)ar->m_func, ar->numArgs(), ar);
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
    DECLARE_FRAME_POINTER(framePtr);
    tl_regState = REGSTATE_CLEAN;
    framePtr->m_savedRip = ar->m_savedRip;
    throw;
  }
}

asm (
  ".byte 0\n"
  ".align 16\n"
  ".globl __funcBodyHelperThunk\n"
"__funcBodyHelperThunk:\n"
#if defined(__x86_64__)
  /*
   * when this helper is called, its as if by a jmp
   * direct from the tc (its actually called by smashing
   * the return address of fCallArrayHelper). So we dont
   * need to worry about stack parity
   */
  "mov %rbp, %rdi\n"
  "call funcBodyHelper\n"
  "jmp *%rax\n"
  "ud2\n"
#elif defined(__AARCH64EL__)
  "brk 0\n"
#else
# error You sure have your work cut out for you
#endif
);

/*
 * This is used to generate an entry point for the entry
 * to a function, after the prologue has run.
 */
TCA funcBodyHelper(ActRec* fp) {
  setupAfterProlog(fp);
  tl_regState = REGSTATE_CLEAN;
  Func* func = const_cast<Func*>(fp->m_func);

  TCA tca = tx64->getCallArrayProlog(func);

  if (tca) {
    func->setFuncBody(tca);
  } else {
    tca = Translator::Get()->getResumeHelper();
  }
  tl_regState = REGSTATE_DIRTY;
  return tca;
}

void TranslatorX64::fCallArrayHelper(const Offset pcOff, const Offset pcNext) {
  DECLARE_FRAME_POINTER(framePtr);
  ActRec* fp = (ActRec*)framePtr->m_savedRbp;

  VMExecutionContext *ec = g_vmContext;
  ec->m_fp = fp;
  ec->m_stack.top() = sp;
  ec->m_pc = curUnit()->at(pcOff);
  PC pc = curUnit()->at(pcNext);

  tl_regState = REGSTATE_CLEAN;
  bool runFunc = ec->doFCallArray(pc);
  sp = ec->m_stack.top();
  tl_regState = REGSTATE_DIRTY;
  if (!runFunc) return;

  ec->m_fp->m_savedRip = framePtr->m_savedRip;
  // smash our return and frame pointer chain
  framePtr->m_savedRip = (uint64_t)ec->m_fp->m_func->getFuncBody();
  framePtr->m_savedRbp = (uint64_t)ec->m_fp;
}

} } } // HPHP::VM::Transl
