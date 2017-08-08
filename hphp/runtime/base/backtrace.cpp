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
#include "hphp/runtime/base/backtrace.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/ext/asio/ext_async-generator-wait-handle.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/util/concurrent-scalable-cache.h"
#include "hphp/util/struct-log.h"

#include <folly/small_vector.h>

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

static c_WaitableWaitHandle* getParentWH(
    c_WaitableWaitHandle* wh,
    context_idx_t contextIdx,
    folly::small_vector<c_WaitableWaitHandle*, 64>& visitedWHs) {
  assertx(!wh->isFinished());
  auto p = wh->getParentChain().firstInContext(contextIdx);
  if (p == nullptr ||
      UNLIKELY(std::find(visitedWHs.begin(), visitedWHs.end(), p)
               != visitedWHs.end())) {
    // If the parent exists in our backtrace, it means we have detected a
    // cycle. We well fall back to savedFP in that case.
    return nullptr;
  }
  visitedWHs.push_back(p);
  return p;
}

// walks up the wait handle dependency chain, until it finds activation record
static ActRec* getActRecFromWaitHandle(
    c_WaitableWaitHandle* currentWaitHandle,
    context_idx_t contextIdx,
    Offset* prevPc,
    folly::small_vector<c_WaitableWaitHandle*, 64>& visitedWHs) {
  while (currentWaitHandle != nullptr) {
    assertx(!currentWaitHandle->isFinished());
    if (currentWaitHandle->getKind() == c_WaitHandle::Kind::AsyncFunction) {
      auto resumable = currentWaitHandle->asAsyncFunction()->resumable();
      *prevPc = resumable->resumeOffset();
      return resumable->actRec();
    }
    if (currentWaitHandle->getKind() == c_WaitHandle::Kind::AsyncGenerator) {
      auto resumable = currentWaitHandle->asAsyncGenerator()->resumable();
      *prevPc = resumable->resumeOffset();
      return resumable->actRec();
    }

    currentWaitHandle = getParentWH(currentWaitHandle, contextIdx, visitedWHs);
  }
  *prevPc = 0;
  return AsioSession::Get()->getContext(contextIdx)->getSavedFP();
}

static ActRec* getPrevActRec(
    const ActRec* fp, Offset* prevPc,
    folly::small_vector<c_WaitableWaitHandle*, 64>& visitedWHs) {
  c_WaitableWaitHandle* currentWaitHandle = nullptr;

  if (fp && fp->func() && fp->resumed()) {
    if (fp->func()->isAsyncFunction()) {
      currentWaitHandle = frame_afwh(fp);
    } else if (fp->func()->isAsyncGenerator() &&
               frame_async_generator(fp)->isRunning()) {
      // getWaitHandle may return null, if generator is executing eagerly
      currentWaitHandle = frame_async_generator(fp)->getWaitHandle();
    }
  }

  if (currentWaitHandle != nullptr) {
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

    currentWaitHandle = getParentWH(currentWaitHandle, contextIdx, visitedWHs);
    return getActRecFromWaitHandle(
      currentWaitHandle, contextIdx, prevPc, visitedWHs);
  }

  return g_context->getPrevVMState(fp, prevPc);
}

// wrapper around getActRecFromWaitHandle, which does some extra validation
static ActRec* getActRecFromWaitHandleWrapper(
    c_WaitableWaitHandle* currentWaitHandle, Offset* prevPc,
    folly::small_vector<c_WaitableWaitHandle*, 64>& visitedWHs) {
  if (currentWaitHandle->isFinished()) {
    return nullptr;
  }

  auto const contextIdx = currentWaitHandle->getContextIdx();
  if (contextIdx <= 0) {
    return nullptr;
  }

  return getActRecFromWaitHandle(
    currentWaitHandle, contextIdx, prevPc, visitedWHs);
}

Array createBacktrace(const BacktraceArgs& btArgs) {
  if (btArgs.isCompact()) {
    return createCompactBacktrace()->extract();
  }

  auto bt = Array::Create();
  folly::small_vector<c_WaitableWaitHandle*, 64> visitedWHs;

  // If there is a parser frame, put it at the beginning of the backtrace.
  if (btArgs.m_parserFrame) {
    bt.append(
      make_map_array(
        s_file, btArgs.m_parserFrame->filename,
        s_line, btArgs.m_parserFrame->lineNumber
      )
    );
  }

  int depth = 0;
  ActRec* fp = nullptr;
  Offset pc = 0;

  if (btArgs.m_fromWaitHandle) {
    fp = getActRecFromWaitHandleWrapper(
      btArgs.m_fromWaitHandle, &pc, visitedWHs);
    // no frames found, we are done
    if (!fp) return bt;
  } else {
    VMRegAnchor _;
    // If there are no VM frames, we're done.
    if (!rds::header() || !vmfp()) return bt;

    // Get the fp and pc of the top frame (possibly skipping one frame).

    if (btArgs.m_skipTop) {
      fp = getPrevActRec(vmfp(), &pc, visitedWHs);
      // We skipped over the only VM frame, we're done.
      if (!fp) return bt;
    } else {
      fp = vmfp();
      auto const unit = fp->func()->unit();
      assert(unit);
      pc = unit->offsetOf(vmpc());
    }

    if (btArgs.m_skipInlined && RuntimeOption::EvalJit) {
      while (fp && (jit::TCA)fp->m_savedRip == jit::tc::ustubs().retInlHelper) {
        fp = getPrevActRec(fp, &pc, visitedWHs);
      }
      if (!fp) return bt;
    }
  }

  // Handle the top frame.
  if (btArgs.m_withSelf) {
    // Builtins don't have a file and line number, so find the first user frame
    auto curFp = fp;
    auto curPc = pc;
    while (curFp && curFp->func()->isBuiltin()) {
      curFp = g_context->getPrevVMState(curFp, &curPc);
    }
    if (curFp) {
      auto const unit = curFp->func()->unit();
      assert(unit);
      auto const filename = curFp->func()->filename();

      ArrayInit frame(btArgs.m_parserFrame ? 4 : 2, ArrayInit::Map{});
      frame.set(s_file, Variant{const_cast<StringData*>(filename)});
      frame.set(s_line, unit->getLineNumber(curPc));
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
  for (auto prevFp = getPrevActRec(fp, &prevPc, visitedWHs);
       fp != nullptr && (btArgs.m_limit == 0 || depth < btArgs.m_limit);
       fp = prevFp, pc = prevPc,
         prevFp = getPrevActRec(fp, &prevPc, visitedWHs)) {
    // Do not capture frame for HPHP only functions.
    if (fp->func()->isNoInjection()) continue;

    ArrayInit frame(8, ArrayInit::Map{});

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
    String funcname{const_cast<StringData*>(fp->func()->displayName())};
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
        if (!isReturning && fp->hasThis()) {
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
    if ((!btArgs.m_withArgNames && !btArgs.m_withArgValues) ||
        !RuntimeOption::EnableArgsInBacktraces) {
      // do nothing
    } else if (funcname.same(s_include)) {
      auto filepath = const_cast<StringData*>(curUnit->filepath());
      frame.set(s_args, make_packed_array(filepath));
    } else if (isReturning) {
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

          auto val = init_null();
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
          if (arg->m_type == KindOfUninit) {
            args.append(init_null());
          } else {
            args.append(tvAsVariant(arg));
          }
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

void addBacktraceToStructLog(const Array& bt, StructuredLogEntry& cols) {
  std::set<std::string> strings;
  std::vector<folly::StringPiece> files;
  std::vector<folly::StringPiece> functions;
  std::vector<folly::StringPiece> lines;
  auto addString = [&] (std::string&& s) -> folly::StringPiece {
    return *strings.insert(std::move(s)).first;
  };
  auto addVariant = [&] (const Variant& v) -> folly::StringPiece {
    if (v.isString()) return v.toCStrRef().slice();
    return addString(v.toString().toCppString());
  };
  for (ArrayIter it(bt.get()); it; ++it) {
    Array frame = it.second().toArray();
    files.emplace_back(addVariant(frame[s_file]));
    if (frame.exists(s_class)) {
      functions.emplace_back(
        addString(folly::sformat("{}{}{}",
                                 frame[s_class].toString().data(),
                                 frame[s_type].toString().data(),
                                 frame[s_function].toString().data()
                                )
                 ));
    } else {
      functions.emplace_back(addVariant(frame[s_function]));
    }
    lines.emplace_back(addVariant(frame[s_line]));
  }
  cols.setVec("php_files", files);
  cols.setVec("php_functions", functions);
  cols.setVec("php_lines", lines);
}

template<class L>
static void walkStack(L func) {
  VMRegAnchor _;
  folly::small_vector<c_WaitableWaitHandle*, 64> visitedWHs;
  ActRec* fp = vmfp();

  // If there are no VM frames, we're done.
  if (!fp || !rds::header()) return;

  // Handle the subsequent VM frames.
  Offset prevPc = 0;
  for (; fp != nullptr; fp = getPrevActRec(fp, &prevPc, visitedWHs)) {

    // Do not capture frame for HPHP only functions.
    if (fp->func()->isNoInjection()) continue;

    func(fp, prevPc);
  }
}

int64_t createBacktraceHash() {
  // Settings constants before looping
  int64_t hash = 0x9e3779b9;
  Unit* prev_unit = nullptr;

  walkStack([&] (ActRec* fp, Offset) {
    auto const curFunc = fp->func();
    auto const curUnit = curFunc->unit();

    // Only do a filehash if the file changed. It is very common
    // to see sequences of calls within the same file
    // File paths are already hashed, and the hash bits are random enough
    // That allows us to do a faster combination of hashes using a known
    // implementation (boost::hash_combine)
    if (prev_unit != curUnit) {
      prev_unit = curUnit;
      auto filehash = curUnit->filepath()->hash();
      hash ^= filehash + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    }

    // Function names are already hashed, and the hash bits are random enough
    // That allows us to do a faster combination of hashes using a known
    // implementation (boost::hash_combine)
    auto funchash = curFunc->fullName()->hash();
    hash ^= funchash + 0x9e3779b9 + (hash << 6) + (hash >> 2);
  });

  return hash;
}

req::ptr<CompactTrace> createCompactBacktrace() {
  auto ret = req::make<CompactTrace>();
  walkStack([&] (ActRec* fp, Offset prevPc) { ret->insert(fp, prevPc); });
  return ret;
}

namespace {

struct CTKHasher final {
  int64_t hash(const CompactTrace::Key& k) const { return k.m_hash; }
  bool equal(const CompactTrace::Key& k1, const CompactTrace::Key& k2) const;
};

struct CacheDeleter final {
  void operator()(ArrayData* ad) const {
    if (!ad->isUncounted()) return;
    Treadmill::enqueue([ad] {
      PackedArray::ReleaseUncounted(ad, 0);
    });
  }
};

using CachedArray = std::shared_ptr<ArrayData>;
using Cache = ConcurrentScalableCache<CompactTrace::Key,CachedArray,CTKHasher>;
Cache s_cache(1024);

bool CTKHasher::equal(
  const CompactTrace::Key& k1,
  const CompactTrace::Key& k2
) const {
  if (k1.m_hash != k2.m_hash || k1.m_frames.size() != k2.m_frames.size()) {
    return false;
  }
  for (int i = 0; i < k1.m_frames.size(); ++i) {
    auto& a = k1.m_frames[i];
    auto& b = k2.m_frames[i];
    if (a.func != b.func || a.prevPcAndHasThis != b.prevPcAndHasThis) {
      return false;
    }
  }
  return true;
}
}

IMPLEMENT_RESOURCE_ALLOCATION(CompactTrace)

void CompactTrace::Key::insert(const ActRec* fp, int32_t prevPc) {
  auto const funcHash = use_lowptr
    ? (uintptr_t)fp->func() << 32
    : (uintptr_t)fp->func();
  m_hash ^= funcHash + 0x9e3779b9 + (m_hash << 6) + (m_hash >> 2);
  m_hash ^= prevPc + 0x9e3779b9 + (m_hash << 6) + (m_hash >> 2);

  auto const curUnit = fp->func()->unit();
  auto const curOp = curUnit->getOp(prevPc);
  auto const isReturning =
    curOp == Op::RetC || curOp == Op::RetV ||
    curOp == Op::CreateCont || curOp == Op::Await ||
    fp->localsDecRefd();
  m_frames.push_back(Frame{
    fp->func(),
    prevPc,
    !isReturning && arGetContextClass(fp) && fp->hasThis()
  });
}

Array CompactTrace::Key::extract() const {
  PackedArrayInit aInit(m_frames.size(), CheckAllocation{});
  for (int idx = 0; idx < m_frames.size(); ++idx) {
    auto const prev = idx < m_frames.size() - 1 ? &m_frames[idx + 1] : nullptr;
    ArrayInit frame(6, ArrayInit::Map{});
    if (prev && !prev->func->isBuiltin()) {
      auto const prevUnit = prev->func->unit();
      auto prevFile = prevUnit->filepath();
      if (prev->func->originalFilename()) {
        prevFile = prev->func->originalFilename();
      }

      auto const prevPc = prev->prevPc;
      auto const opAtPrevPc = prevUnit->getOp(prevPc);
      Offset pcAdjust = 0;
      if (opAtPrevPc == Op::PopR ||
          opAtPrevPc == Op::UnboxR ||
          opAtPrevPc == Op::UnboxRNop) {
        pcAdjust = 1;
      }
      frame.set(s_file, StrNR(prevFile).asString());
      frame.set(s_line, prevUnit->getLineNumber(prevPc - pcAdjust));
    }

    auto const f = m_frames[idx].func;

    // Check for include.
    String funcname{const_cast<StringData*>(f->displayName())};
    if (f->isClosureBody()) {
      // Strip the file hash from the closure name.
      String fullName{const_cast<StringData*>(f->baseCls()->name())};
      funcname = fullName.substr(0, fullName.find(';'));
    }

    // Check for pseudomain.
    if (funcname.empty()) {
      if (!prev) continue;
      else funcname = s_include;
    }

    frame.set(s_function, funcname);

    if (!funcname.same(s_include)) {
      // Closures have an m_this but they aren't in object context.
      auto ctx = m_frames[idx].func->cls();
      if (ctx != nullptr && !f->isClosureBody()) {
        frame.set(s_class, Variant{const_cast<StringData*>(ctx->name())});
        if (m_frames[idx].hasThis) {
          frame.set(s_type, s_arrow);
        } else {
          frame.set(s_type, s_double_colon);
        }
      }
    } else {
      auto filepath = const_cast<StringData*>(f->unit()->filepath());
      frame.set(s_args, make_packed_array(filepath));
    }

    aInit.append(frame.toVariant());
  }

  return aInit.toArray();
}

Array CompactTrace::extract() const {
  if (m_key.m_frames.size() == 1) return empty_array();

  Cache::ConstAccessor acc;
  if (s_cache.find(acc, m_key)) {
    return Array(acc.get()->get());
  }

  auto arr = m_key.extract();
  auto ins = CachedArray(
    arr.get()->empty()
      ? staticEmptyArray()
      : PackedArray::MakeUncounted(arr.get(), 0),
    CacheDeleter()
  );
  if (!s_cache.insert(m_key, ins)) {
    return arr;
  }
  return Array(ins.get());
}

} // HPHP
