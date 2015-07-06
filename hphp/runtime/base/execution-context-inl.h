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

#ifndef incl_HPHP_EXECUTION_CONTEXT_INL_H_
#define incl_HPHP_EXECUTION_CONTEXT_INL_H_

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline void* ExecutionContext::operator new(size_t s) {
  return req::malloc(s);
}

inline void* ExecutionContext::operator new(size_t s, void* p) {
  return p;
}

inline void ExecutionContext::operator delete(void* p) {
  req::free(p);
}

inline Transport* ExecutionContext::getTransport() {
  return m_transport;
}

inline void ExecutionContext::setTransport(Transport* transport) {
  m_transport = transport;
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

inline Array ExecutionContext::getEnvs() const {
  return m_envs;
}

inline String ExecutionContext::getTimeZone() const {
  return m_timezone;
}

inline void ExecutionContext::setTimeZone(const String& timezone) {
  m_timezone = timezone;
}

inline String ExecutionContext::getDefaultTimeZone() const {
  return m_timezoneDefault;
}

inline void ExecutionContext::setDefaultTimeZone(const String& timezone) {
  m_timezoneDefault = timezone;
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

inline const Func* ExecutionContext::getPrevFunc(const ActRec* fp) {
  auto state = getPrevVMState(fp, nullptr, nullptr, nullptr);
  return state ? state->func() : nullptr;
}

inline void ExecutionContext::invokeFunc(
  TypedValue* retval,
  const CallCtx& ctx,
  const Variant& args_,
  VarEnv* varEnv
) {
  invokeFunc(retval, ctx.func, args_, ctx.this_, ctx.cls, varEnv, ctx.invName);
}

inline void ExecutionContext::invokeFuncFew(
  TypedValue* retval,
  const Func* f,
  void* thisOrCls,
  StringData* invName
) {
  invokeFuncFew(retval, f, thisOrCls, invName, 0, nullptr);
}

inline void ExecutionContext::invokeFuncFew(
  TypedValue* retval,
  const CallCtx& ctx,
  int argc,
  const TypedValue* argv
) {
  auto const thisOrCls = [&] () -> void* {
    if (ctx.this_) return (void*)(ctx.this_);
    if (ctx.cls) return (void*)((char*)(ctx.cls) + 1);
    return nullptr;
  }();

  invokeFuncFew(
    retval,
    ctx.func,
    thisOrCls,
    ctx.invName,
    argc,
    argv
  );
}

inline TypedValue ExecutionContext::invokeMethod(
  ObjectData* obj,
  const Func* meth,
  InvokeArgs args
) {
  TypedValue ret;
  invokeFuncFew(
    &ret,
    meth,
    ActRec::encodeThis(obj),
    nullptr /* invName */,
    args.size(),
    args.start()
  );
  return ret;
}

inline Variant ExecutionContext::invokeMethodV(
  ObjectData* obj,
  const Func* meth,
  InvokeArgs args
) {
  auto const tv = invokeMethod(obj, meth, args);

  // Construct variant without triggering incref.
  Variant ret;
  *ret.asTypedValue() = tv;
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}

#endif
