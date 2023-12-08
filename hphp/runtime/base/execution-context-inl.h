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

#include "hphp/runtime/vm/act-rec.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

inline void* ExecutionContext::operator new(size_t s) {
  // Can't use req::make_raw here because we want raw memory, not a constructed
  // object. This gets called generically from ThreadLocal, so we can't just
  // change the call-sites.
  return req::malloc(s, type_scan::getIndexForMalloc<ExecutionContext>());
}

inline void* ExecutionContext::operator new(size_t /*s*/, void* p) {
  return p;
}

inline void ExecutionContext::operator delete(void* p) {
  req::free(p);
}

inline Transport* ExecutionContext::getTransport() {
  return m_transport;
}

// This method may return different implementations of StreamTransport
// based on runtime options.
inline std::shared_ptr<stream_transport::StreamTransport>
ExecutionContext::getServerStreamTransport() const {
  if (m_transport) {
    return m_transport->getStreamTransport();
  } else {
    return nullptr;
  }
}

inline rqtrace::Trace* ExecutionContext::getRequestTrace() {
  return m_requestTrace;
}

inline void ExecutionContext::setTransport(Transport* transport) {
  m_transport = transport;
  if (transport && !m_requestTrace) {
    if (auto trace = m_transport->getRequestTrace()) m_requestTrace = trace;
  }
}

inline void ExecutionContext::setRequestTrace(rqtrace::Trace* trace) {
  m_requestTrace = trace;
}

inline String ExecutionContext::getCwd() const {
  return m_cwd;
}

inline void ExecutionContext::setCwd(const String& cwd) {
  m_cwd = cwd;
}

inline void ExecutionContext::write(const char* s) {
  write(s, strlen(s));
}

inline StringBuffer* ExecutionContext::swapOutputBuffer(StringBuffer* sb) {
  // If we are swapping output buffers (currently done by the debugger)
  // then any current chunking is off the table
  if (m_out != nullptr) {
    if (sb != &m_out->oss) {
      m_remember_chunk =  m_out->chunk_size;
      m_out->chunk_size = 0;
    } else if (sb == &m_out->oss) { // pointing to same thing,swapping back in
      m_out->chunk_size = m_remember_chunk;
    }
  }
  auto current = m_sb;
  m_sb = sb;
  return current;
}

inline String ExecutionContext::getRawPostData() const {
  return m_rawPostData;
}

inline void ExecutionContext::setRawPostData(const String& pd) {
  m_rawPostData = pd;
}

inline ExecutionContext::ErrorState ExecutionContext::getErrorState() const {
  return m_errorState;
}

inline void ExecutionContext::setErrorState(
  ExecutionContext::ErrorState state
) {
  m_errorState = state;
}

inline String ExecutionContext::getLastError() const {
  return m_lastError;
}

inline int ExecutionContext::getLastErrorNumber() const {
  return m_lastErrorNum;
}

inline String ExecutionContext::getErrorPage() const {
  return m_errorPage;
}

inline void ExecutionContext::setErrorPage(const String& page) {
  m_errorPage = page;
}

inline String ExecutionContext::getLastErrorPath() const {
  return m_lastErrorPath;
}

inline int ExecutionContext::getLastErrorLine() const {
  return m_lastErrorLine;
}

inline Array ExecutionContext::releaseDeferredErrors() {
  auto ret = std::move(m_deferredErrors);
  m_deferredErrors = Array::CreateVec();
  return ret;
}

inline Array ExecutionContext::getEnvs() const {
  return m_envs;
}

inline String ExecutionContext::getTimezone() const {
  return m_timezone;
}

inline void ExecutionContext::setTimezone(const String& timezone) {
  m_timezone = timezone;
}

inline bool ExecutionContext::getThrowAllErrors() const {
  return m_throwAllErrors;
}

inline void ExecutionContext::setThrowAllErrors(bool throwAllErrors) {
  m_throwAllErrors = throwAllErrors;
}

inline Variant ExecutionContext::getExitCallback() {
  return m_exitCallback;
}

inline void ExecutionContext::setExitCallback(Variant callback) {
  m_exitCallback = callback;
}

inline void
ExecutionContext::setStreamContext(const req::ptr<StreamContext>& context) {
  m_streamContext = context;
}

inline const req::ptr<StreamContext>& ExecutionContext::getStreamContext() {
  return m_streamContext;
}

inline int ExecutionContext::getPageletTasksStarted() const {
  return m_pageletTasksStarted;
}

inline void ExecutionContext::incrPageletTasksStarted() {
  ++m_pageletTasksStarted;
}

inline const VirtualHost* ExecutionContext::getVirtualHost() const {
  return m_vhost;
}

inline void ExecutionContext::setVirtualHost(const VirtualHost* vhost) {
  m_vhost = vhost;
}

inline const String& ExecutionContext::getSandboxId() const {
  return m_sandboxId;
}

inline void ExecutionContext::setSandboxId(const String& sandboxId) {
  m_sandboxId = sandboxId;
}

inline bool ExecutionContext::hasRequestEventHandlers() const {
  return !m_requestEventHandlers.empty();
}

inline const RepoOptions* ExecutionContext::getRepoOptionsForRequest() const {
  return m_requestOptions.get_pointer();
}

inline const PackageInfo& ExecutionContext::getPackageInfo() const {
  if (RO::RepoAuthoritative) return RepoFile::packageInfo();
  if (auto const opts = getRepoOptionsForRequest()) {
    return opts->packageInfo();
  }
  raise_error("Unable to retrieve package information");
}

inline const Func* ExecutionContext::getPrevFunc(const ActRec* fp) {
  auto state = getPrevVMState(fp, nullptr, nullptr, nullptr);
  return state ? state->func() : nullptr;
}

inline TypedValue ExecutionContext::invokeFunc(
  const CallCtx& ctx,
  const Variant& args_,
  RuntimeCoeffects providedCoeffects
) {
  return invokeFunc(ctx.func, args_, ctx.this_, ctx.cls,
                    providedCoeffects, ctx.dynamic);
}

inline TypedValue ExecutionContext::invokeFuncFew(
  const Func* f,
  ExecutionContext::ThisOrClass thisOrCls,
  RuntimeCoeffects providedCoeffects
) {
  return invokeFuncFew(f, thisOrCls, 0, nullptr, providedCoeffects);
}

inline TypedValue ExecutionContext::invokeFuncFew(
  const CallCtx& ctx,
  uint32_t numArgs,
  const TypedValue* argv,
  RuntimeCoeffects providedCoeffects
) {
  auto const thisOrCls = [&] () -> ExecutionContext::ThisOrClass {
    if (ctx.this_) return ctx.this_;
    if (ctx.cls) return ctx.cls;
    return nullptr;
  }();

  return invokeFuncFew(
    ctx.func,
    thisOrCls,
    numArgs,
    argv,
    providedCoeffects,
    ctx.dynamic
  );
}

inline TypedValue ExecutionContext::invokeMethod(
  ObjectData* obj,
  const Func* meth,
  InvokeArgs args,
  RuntimeCoeffects providedCoeffects
) {
  return invokeFuncFew(
    meth,
    obj,
    args.size(),
    args.start(),
    providedCoeffects,
    false,
    false
  );
}

inline Variant ExecutionContext::invokeMethodV(
  ObjectData* obj,
  const Func* meth,
  InvokeArgs args,
  RuntimeCoeffects providedCoeffects
) {
  // Construct variant without triggering incref.
  return Variant::attach(invokeMethod(obj, meth, args, providedCoeffects));
}

inline ActRec* ExecutionContext::getOuterVMFrame(const ActRec* ar) {
  ActRec* sfp = ar->sfp();
  if (LIKELY(sfp != nullptr)) return sfp;
  return LIKELY(!m_nestedVMs.empty()) ? m_nestedVMs.back().fp : nullptr;
}

inline TypedValue ExecutionContext::lookupClsCns(const StringData* cls,
                                      const StringData* cns) {
  return lookupClsCns(NamedType::getOrCreate(cls), cls, cns);
}

inline ActRec*
ExecutionContext::getPrevVMStateSkipFrame(const ActRec* fp,
                                          Offset* prevPc /* = NULL */,
                                          TypedValue** prevSp /* = NULL */,
                                          bool* fromVMEntry /* = NULL */) {
  auto prev = getPrevVMState(fp, prevPc, prevSp, fromVMEntry);
  if (LIKELY(!prev || !prev->skipFrame())) return prev;
  do {
    prev = getPrevVMState(prev, prevPc, prevSp, fromVMEntry);
  } while (prev && prev->skipFrame());
  return prev;
}

template<class Fn> void ExecutionContext::sweepDynPropTable(Fn fn) {
  for (auto i = dynPropTable.begin(); i != dynPropTable.end();) {
    if (fn(i->first)) {
      i = dynPropTable.erase(i);
    } else {
      ++i;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

}
