/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/runtime/vm/bytecode.h"

namespace HPHP {

const StaticString s_file("file");
const StaticString s_line("line");
const StaticString s_function("function");
const StaticString s_args("args");
const StaticString s_class("class");
const StaticString s_object("object");
const StaticString s_type("type");
const StaticString s_include("include");


Array createBacktrace(const BacktraceArgs& btArgs) {
  Array bt = Array::Create();

  // If there is a parser frame, put it at the beginning of
  // the backtrace
  if (btArgs.m_parserFrame) {
    bt.append(
      make_map_array(
        s_file, btArgs.m_parserFrame->filename,
        s_line, btArgs.m_parserFrame->lineNumber
      )
    );
  }

  VMRegAnchor _;
  if (!vmfp()) {
    // If there are no VM frames, we're done
    return bt;
  }

  int depth = 0;
  ActRec* fp = nullptr;
  Offset pc = 0;

  // Get the fp and pc of the top frame (possibly skipping one frame)
  {
    if (btArgs.m_skipTop) {
      fp = g_context->getPrevVMState(vmfp(), &pc);
      if (!fp) {
        // We skipped over the only VM frame, we're done
        return bt;
      }
    } else {
      fp = vmfp();
      Unit *unit = vmfp()->m_func->unit();
      assert(unit);
      pc = unit->offsetOf(vmpc());
    }

    // Handle the top frame
    if (btArgs.m_withSelf) {
      // Builtins don't have a file and line number
      if (!fp->m_func->isBuiltin()) {
        Unit* unit = fp->m_func->unit();
        assert(unit);
        const char* filename = fp->m_func->filename()->data();
        Offset off = pc;

        ArrayInit frame(btArgs.m_parserFrame ? 4 : 2, ArrayInit::Map{});
        frame.set(s_file, filename);
        frame.set(s_line, unit->getLineNumber(off));
        if (btArgs.m_parserFrame) {
          frame.set(s_function, s_include);
          frame.set(s_args, Array::Create(btArgs.m_parserFrame->filename));
        }
        bt.append(frame.toVariant());
        depth++;
      }
    }
  }
  // Handle the subsequent VM frames
  Offset prevPc = 0;
  for (ActRec* prevFp = g_context->getPrevVMState(fp, &prevPc);
       fp != nullptr && (btArgs.m_limit == 0 || depth < btArgs.m_limit);
       fp = prevFp, pc = prevPc,
         prevFp = g_context->getPrevVMState(fp, &prevPc)) {
    // do not capture frame for HPHP only functions
    if (fp->m_func->isNoInjection()) {
      continue;
    }

    ArrayInit frame(7, ArrayInit::Map{});

    auto const curUnit = fp->m_func->unit();
    auto const curOp = *reinterpret_cast<const Op*>(curUnit->at(pc));
    auto const isReturning =
      curOp == Op::RetC || curOp == Op::RetV ||
      curOp == Op::CreateCont || curOp == Op::Await ||
      fp->localsDecRefd();

    // Builtins and generators don't have a file and line number
    if (prevFp && !prevFp->m_func->isBuiltin() && !fp->resumed()) {
      auto const prevUnit = prevFp->m_func->unit();
      auto prevFile = prevUnit->filepath();
      if (prevFp->m_func->originalFilename()) {
        prevFile = prevFp->m_func->originalFilename();
      }
      assert(prevFile);
      frame.set(s_file, const_cast<StringData*>(prevFile));

      // In the normal method case, the "saved pc" for line number printing is
      // pointing at the cell conversion (Unbox/Pop) instruction, not the call
      // itself. For multi-line calls, this instruction is associated with the
      // subsequent line which results in an off-by-n. We're subtracting one
      // in order to look up the line associated with the FCall/FCallArray
      // instruction. Exception handling and the other opcodes (ex. BoxR)
      // already do the right thing. The emitter associates object access with
      // the subsequent expression and this would be difficult to modify.
      auto const opAtPrevPc =
        *reinterpret_cast<const Op*>(prevUnit->at(prevPc));
      Offset pcAdjust = 0;
      if (opAtPrevPc == OpPopR || opAtPrevPc == OpUnboxR) {
        pcAdjust = 1;
      }
      frame.set(s_line,
                prevFp->m_func->unit()->getLineNumber(prevPc - pcAdjust));
    }

    // check for include
    String funcname = const_cast<StringData*>(fp->m_func->name());
    if (fp->m_func->isClosureBody()) {
      static StringData* s_closure_label =
        makeStaticString("{closure}");
      funcname = s_closure_label;
    }

    // check for pseudomain
    if (funcname.empty()) {
      if (!prevFp) continue;
      funcname = s_include;
    }

    frame.set(s_function, funcname);

    if (!funcname.same(s_include)) {
      // Closures have an m_this but they aren't in object context
      Class* ctx = arGetContextClass(fp);
      if (ctx != nullptr && !fp->m_func->isClosureBody()) {
        frame.set(s_class, ctx->name()->data());
        if (fp->hasThis() && !isReturning) {
          if (btArgs.m_withThis) {
            frame.set(s_object, Object(fp->getThis()));
          }
          frame.set(s_type, "->");
        } else {
          frame.set(s_type, "::");
        }
      }
    }

    Array args = Array::Create();
    if (btArgs.m_ignoreArgs) {
      // do nothing
    } else if (funcname.same(s_include)) {
      if (depth) {
        args.append(const_cast<StringData*>(curUnit->filepath()));
        frame.set(s_args, args);
      }
    } else if (!RuntimeOption::EnableArgsInBacktraces || isReturning) {
      // Provide an empty 'args' array to be consistent with hphpc
      frame.set(s_args, args);
    } else {
      const int nparams = fp->m_func->numNonVariadicParams();
      int nargs = fp->numArgs();
      int nformals = std::min(nparams, nargs);

      if (UNLIKELY(fp->hasVarEnv() && fp->getVarEnv()->getFP() != fp)) {
        // VarEnv is attached to eval or debugger frame, other than the current
        // frame. Access locals thru VarEnv.
        auto varEnv = fp->getVarEnv();
        auto func = fp->func();
        for (int i = 0; i < nformals; i++) {
          TypedValue *arg = varEnv->lookup(func->localVarName(i));
          args.append(tvAsVariant(arg));
        }
      } else {
        for (int i = 0; i < nformals; i++) {
          TypedValue *arg = frame_local(fp, i);
          args.append(tvAsVariant(arg));
        }
      }

      /* builtin extra args are not stored in varenv */
      if (nargs > nparams && fp->hasExtraArgs()) {
        for (int i = nparams; i < nargs; i++) {
          TypedValue *arg = fp->getExtraArg(i - nparams);
          args.append(tvAsVariant(arg));
        }
      }
      frame.set(s_args, args);
    }

    bt.append(frame.toVariant());
    depth++;
  }
  return bt;

}


} // HPHP
