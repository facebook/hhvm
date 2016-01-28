/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/backtrace.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {

const StaticString
  s_file("file"),
  s_line("line"),
  s_function("function"),
  s_args("args"),
  s_class("class"),
  s_object("object"),
  s_type("type"),
  s_include("include"),
  s_main("{main}"),
  s_metadata("metadata"),
  s_86metadata("86metadata"),
  s_arrow("->"),
  s_double_colon("::");

static ActRec* getPrevActRec(const ActRec* fp, Offset* prevPc) {
  if (fp && fp->func() && fp->resumed() && fp->func()->isAsyncFunction()) {
    c_WaitableWaitHandle* currentWaitHandle = frame_afwh(fp);
    if (currentWaitHandle->isFinished()) {
      /*
       * It's possible in very rare cases (it will return a truncated stack):
       * 1) async function which WaitHandle is not referenced by anything
       *      else finishes
       * 2) its return value is an object with destructor
       * 3) this destructor gets called as part of destruction of the
       *      WaitHandleobject, which happens right before FP is adjusted
      */
      return nullptr;
    }

    auto const contextIdx = currentWaitHandle->getContextIdx();
    while (currentWaitHandle != nullptr) {
      auto p = currentWaitHandle->getParentChain().firstInContext(contextIdx);
      if (p == nullptr) break;

      if (p->getKind() == c_WaitHandle::Kind::AsyncFunction) {
        auto wh = p->asAsyncFunction();
        *prevPc = wh->resumable()->resumeOffset();
        return wh->actRec();
      }
      currentWaitHandle = p;
    }
    *prevPc = 0;
    return AsioSession::Get()->getContext(contextIdx)->getSavedFP();
  }
  return g_context->getPrevVMState(fp, prevPc);
}

Array createBacktrace(const BacktraceArgs& btArgs) {
  auto bt = Array::Create();

  // If there is a parser frame, put it at the beginning of the backtrace.
  if (btArgs.m_parserFrame) {
    bt.append(
      make_map_array(
        s_file, btArgs.m_parserFrame->filename,
        s_line, btArgs.m_parserFrame->lineNumber
      )
    );
  }

  VMRegAnchor _;
  // If there are no VM frames, we're done.
  if (!rds::header() || !vmfp()) return bt;

  int depth = 0;
  ActRec* fp = nullptr;
  Offset pc = 0;

  // Get the fp and pc of the top frame (possibly skipping one frame).

  if (btArgs.m_skipTop) {
    fp = getPrevActRec(vmfp(), &pc);
    // We skipped over the only VM frame, we're done.
    if (!fp) return bt;
  } else {
    fp = vmfp();
    auto const unit = fp->func()->unit();
    assert(unit);
    pc = unit->offsetOf(vmpc());
  }

  // Handle the top frame.
  if (btArgs.m_withSelf) {
    // Builtins don't have a file and line number.
    if (!fp->func()->isBuiltin()) {
      auto const unit = fp->func()->unit();
      assert(unit);
      auto const filename = fp->func()->filename();

      ArrayInit frame(btArgs.m_parserFrame ? 4 : 2, ArrayInit::Map{});
      frame.set(s_file, Variant{const_cast<StringData*>(filename)});
      frame.set(s_line, unit->getLineNumber(pc));
      if (btArgs.m_parserFrame) {
        frame.set(s_function, s_include);
        frame.set(s_args, Array::Create(btArgs.m_parserFrame->filename));
      }
      bt.append(frame.toVariant());
      depth++;
    }
  }

  // Handle the subsequent VM frames.
  Offset prevPc = 0;
  for (auto prevFp = getPrevActRec(fp, &prevPc);
       fp != nullptr && (btArgs.m_limit == 0 || depth < btArgs.m_limit);
       fp = prevFp, pc = prevPc,
         prevFp = getPrevActRec(fp, &prevPc)) {
    // Do not capture frame for HPHP only functions.
    if (fp->func()->isNoInjection()) continue;

    ArrayInit frame(7, ArrayInit::Map{});

    auto const curUnit = fp->func()->unit();
    auto const curOp = curUnit->getOp(pc);
    auto const isReturning =
      curOp == Op::RetC || curOp == Op::RetV ||
      curOp == Op::CreateCont || curOp == Op::Await ||
      fp->localsDecRefd();

    // Builtins and generators don't have a file and line number
    if (prevFp && !prevFp->func()->isBuiltin()) {
      auto const prevUnit = prevFp->func()->unit();
      auto prevFile = prevUnit->filepath();
      if (prevFp->func()->originalFilename()) {
        prevFile = prevFp->func()->originalFilename();
      }
      assert(prevFile);
      frame.set(s_file, Variant{const_cast<StringData*>(prevFile)});

      // In the normal method case, the "saved pc" for line number printing is
      // pointing at the cell conversion (Unbox/Pop) instruction, not the call
      // itself. For multi-line calls, this instruction is associated with the
      // subsequent line which results in an off-by-n. We're subtracting one
      // in order to look up the line associated with the FCall/FCallArray
      // instruction. Exception handling and the other opcodes (ex. BoxR)
      // already do the right thing. The emitter associates object access with
      // the subsequent expression and this would be difficult to modify.
      auto const opAtPrevPc = prevUnit->getOp(prevPc);
      Offset pcAdjust = 0;
      if (opAtPrevPc == Op::PopR ||
          opAtPrevPc == Op::UnboxR ||
          opAtPrevPc == Op::UnboxRNop) {
        pcAdjust = 1;
      }
      frame.set(s_line,
                prevFp->func()->unit()->getLineNumber(prevPc - pcAdjust));
    }

    // Check for include.
    String funcname{const_cast<StringData*>(fp->func()->name())};
    if (fp->func()->isClosureBody()) {
      // Strip the file hash from the closure name.
      String fullName{const_cast<StringData*>(fp->func()->baseCls()->name())};
      funcname = fullName.substr(0, fullName.find(';'));
    }

    // Check for pseudomain.
    if (funcname.empty()) {
      if (!prevFp && !btArgs.m_withPseudoMain) continue;
      else if (!prevFp) funcname = s_main;
      else funcname = s_include;
    }

    frame.set(s_function, funcname);

    if (!funcname.same(s_include)) {
      // Closures have an m_this but they aren't in object context.
      auto ctx = arGetContextClass(fp);
      if (ctx != nullptr && !fp->func()->isClosureBody()) {
        frame.set(s_class, Variant{const_cast<StringData*>(ctx->name())});
        if (fp->hasThis() && !isReturning) {
          if (btArgs.m_withThis) {
            frame.set(s_object, Object(fp->getThis()));
          }
          frame.set(s_type, s_arrow);
        } else {
          frame.set(s_type, s_double_colon);
        }
      }
    }

    bool const mayUseVV = fp->func()->attrs() & AttrMayUseVV;

    auto const withNames = btArgs.m_withArgNames;
    auto const withValues = btArgs.m_withArgValues;
    if (!btArgs.m_withArgNames && !btArgs.m_withArgValues) {
      // do nothing
    } else if (funcname.same(s_include)) {
      if (depth != 0) {
        auto filepath = const_cast<StringData*>(curUnit->filepath());
        frame.set(s_args, make_packed_array(filepath));
      }
    } else if (!RuntimeOption::EnableArgsInBacktraces || isReturning) {
      // Provide an empty 'args' array to be consistent with hphpc.
      frame.set(s_args, empty_array());
    } else {
      auto args = Array::Create();
      auto const nparams = fp->func()->numNonVariadicParams();
      auto const nargs = fp->numArgs();
      auto const nformals = std::min<int>(nparams, nargs);

      if (UNLIKELY(mayUseVV) &&
          UNLIKELY(fp->hasVarEnv() && fp->getVarEnv()->getFP() != fp)) {
        // VarEnv is attached to eval or debugger frame, other than the current
        // frame. Access locals thru VarEnv.
        auto varEnv = fp->getVarEnv();
        auto func = fp->func();
        for (int i = 0; i < nformals; i++) {
          auto const argname = func->localVarName(i);
          auto const tv = varEnv->lookup(argname);

          Variant val;
          if (tv != nullptr) { // the variable hasn't been unset
            val = withValues ? tvAsVariant(tv) : "";
          }

          if (withNames) {
            args.set(String(const_cast<StringData*>(argname)), val);
          } else {
            args.append(val);
          }
        }
      } else {
        for (int i = 0; i < nformals; i++) {
          Variant val = withValues ? tvAsVariant(frame_local(fp, i)) : "";

          if (withNames) {
            auto const argname = fp->func()->localVarName(i);
            args.set(String(const_cast<StringData*>(argname)), val);
          } else {
            args.append(val);
          }
        }
      }

      // Builtin extra args are not stored in varenv.
      if (UNLIKELY(mayUseVV) && nargs > nparams && fp->hasExtraArgs()) {
        for (int i = nparams; i < nargs; i++) {
          auto arg = fp->getExtraArg(i - nparams);
          args.append(tvAsVariant(arg));
        }
      }
      frame.set(s_args, args);
    }

    if (btArgs.m_withMetadata && !isReturning) {
      if (UNLIKELY(mayUseVV) && UNLIKELY(fp->hasVarEnv())) {
        auto tv = fp->getVarEnv()->lookup(s_86metadata.get());
        if (tv != nullptr && tv->m_type != KindOfUninit) {
          frame.set(s_metadata, tvAsVariant(tv));
        }
      } else {
        auto local = fp->func()->lookupVarId(s_86metadata.get());
        if (local != kInvalidId) {
          auto tv = frame_local(fp, local);
          if (tv->m_type != KindOfUninit) {
            frame.set(s_metadata, tvAsVariant(tv));
          }
        }
      }
    }

    bt.append(frame.toVariant());
    depth++;
  }

  return bt;
}


} // HPHP
