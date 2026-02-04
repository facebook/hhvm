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

#include "hphp/util/trace.h"

namespace HPHP::jit::mcgen {

namespace detail {

/*
 * Check that async translation of `sk' for a live translation kind is allowed,
 * and if so mark the current thread as having enqueued a translation request.
 *
 * !! If this function returns true the caller *must* call the enqueue function.
 */
bool mayEnqueueAsyncTranslateRequest(const SrcKey& sk);

/*
 * Enqueue a translation of `kind' for `ctx'. You must check
 * mayEnqueueAsyncTranslateRequest before calling this function.
 */
void enqueueAsyncTranslateRequest(TransKind kind, RegionContext&& ctx,
                                  int currNumTranslations);
}

/*
 * Check that the async JIT can be used for a translation of type `kind'
 */
bool isAsyncJitEnabled(TransKind kind);

/*
 * Attempt to enqueue a request for a translation of `kind' using `getCtx' to
 * acquire a RegionContext lazily. If another request is currently in the queue
 * or the number of retranslations for the source key is not equal to
 * currNumTranslations this request will be silently dropped.
 *
 * We need to avoid attempting to create the same translation multiple times, so
 * if another request has raced with ours we will need to try again after later
 * to confirm that a ReqRetranslate from any new translations is actually
 * required.
 */
template<class F>
void enqueueAsyncTranslateRequest(TransKind kind, const SrcKey& sk,
                                    F&& getCtx, int currNumTranslations) {
  if (!Cfg::Eval::AsyncJitDeferContext) {
    auto ctx = getCtx();
    if (detail::mayEnqueueAsyncTranslateRequest(sk)) {
      detail::enqueueAsyncTranslateRequest(kind, std::move(ctx),
                                           currNumTranslations);
      return;
    }
  } else if (detail::mayEnqueueAsyncTranslateRequest(sk)) {
    detail::enqueueAsyncTranslateRequest(kind, getCtx(), currNumTranslations);
    return;
  }

  FTRACE_MOD(Trace::async_jit, 2,
    "In progress jitting found, skipping enqueue for sk {}\n",
    show(sk)
  );
}

/*
 * Request an async translation of `kind' for the prologue of `func' for
 * `nPassed' parameters. Ensures only one copy of each prologue is translated.
 */
void enqueueAsyncPrologueRequest(TransKind kind, Func* func, int nPassed);

/*
 * During sandbox jumpstart we need to enqueue deserialized region contexts for
 * translation. We want to enqueue all contexts at once, and we know that the
 * serialized contexts should not conflict with each other, so this bypasses the
 * usual checks for enqueuing tranlsations.
 */
void enqueueAsyncTranslateRequestForJumpstart(RegionContext&& ctx);

/*
 * Request a prologue be enqueued for translation as part of sandbox mode
 * jumpstart, also bypasses any checks for duplicate prologues.
 */
void enqueueAsyncPrologueRequestForJumpstart(Func* func, int nPassed);

/*
 * Enqueue optimization of `func' on the async JIT. Ensures that each function
 * is optimized only once.
 */
void enqueueAsyncTranslateOptRequest(const Func* func);

/*
 * Wait for background async JIT thread pool to terminate.
 */
void joinAsyncTranslationWorkerThreads();

}
