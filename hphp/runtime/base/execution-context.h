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

#ifndef incl_HPHP_EXECUTION_CONTEXT_H_
#define incl_HPHP_EXECUTION_CONTEXT_H_

#include <list>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "hphp/runtime/base/apc-handle.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/server/transport.h"
#include "hphp/runtime/server/virtual-host.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/minstr-state.h"
#include "hphp/runtime/vm/pc-filter.h"
#include "hphp/util/lock.h"
#include "hphp/util/thread-local.h"

namespace vixl { class Simulator; }

namespace HPHP {
struct RequestEventHandler;
struct EventHook;
struct Resumable;
struct PhpFile;
namespace jit { struct Translator; }
}

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct VMState {
  PC pc;
  ActRec* fp;
  ActRec* firstAR;
  TypedValue* sp;
  MInstrState mInstrState;
};

enum class CallType {
  ClsMethod,
  ObjMethod,
  CtorMethod,
};
enum class LookupResult {
  MethodFoundWithThis,
  MethodFoundNoThis,
  MagicCallFound,
  MagicCallStaticFound,
  MethodNotFound,
};

enum class InclOpFlags {
  Default = 0,
  Fatal = 1,
  Once = 2,
  DocRoot = 8,
  Relative = 16,
};

inline InclOpFlags operator|(const InclOpFlags& l, const InclOpFlags& r) {
  return static_cast<InclOpFlags>(static_cast<int>(l) | static_cast<int>(r));
}

inline bool operator&(const InclOpFlags& l, const InclOpFlags& r) {
  return static_cast<int>(l) & static_cast<int>(r);
}

struct VMParserFrame {
  std::string filename;
  int lineNumber;
};

struct DebuggerSettings {
  bool bypassCheck = false;
  bool stackArgs = true;
  int printLevel = -1;
};

using InvokeArgs = folly::Range<const TypedValue*>;

///////////////////////////////////////////////////////////////////////////////

struct ExecutionContext {
  enum ShutdownType {
    ShutDown,
    PostSend,
  };

  enum class ErrorThrowMode {
    Never,
    IfUnhandled,
    Always,
  };

  enum class ErrorState {
    NoError,
    ErrorRaised,
    ExecutingUserHandler,
    ErrorRaisedByUserHandler,
  };

public:
  ExecutionContext();
  ExecutionContext(const ExecutionContext&) = delete;
  ExecutionContext& operator=(const ExecutionContext&) = delete;
  ~ExecutionContext();
  void sweep();

  void* operator new(size_t s);
  void* operator new(size_t s, void* p);
  void operator delete(void* p);

  // For RPCRequestHandler.
  void backupSession();
  void restoreSession();

  /*
   * API for the debugger.  Format of the vector is the same as
   * IDebuggable::debuggerInfo, but we don't actually need to
   * implement that interface since the execution context is not
   * accessed by the debugger polymorphically.
   */
  void debuggerInfo(std::vector<std::pair<const char*,std::string>>&);

  /**
   * System settings.
   */
  Transport* getTransport();
  void setTransport(Transport*);
  std::string getRequestUrl(size_t szLimit = std::string::npos);
  String getMimeType() const;
  void setContentType(const String& mimetype, const String& charset);
  String getCwd() const;
  void setCwd(const String&);

  /**
   * Write to output.
   */
  void write(const String&);
  void write(const char* s, int len);
  void write(const char*);

  void writeStdout(const char* s, int len);
  size_t getStdoutBytesWritten() const;

  using PFUNC_STDOUT = void (*)(const char* s, int len, void* data);
  void setStdout(PFUNC_STDOUT func, void* data);

  /**
   * Output buffering.
   */
  void obStart(const Variant& handler = uninit_null(), int chunk_size = 0);
  String obCopyContents();
  String obDetachContents();
  int obGetContentLength();
  void obClean(int handler_flag);
  bool obFlush();
  void obFlushAll();
  bool obEnd();
  void obEndAll();
  int obGetLevel();
  Array obGetStatus(bool full);
  void obSetImplicitFlush(bool on);
  Array obGetHandlers();
  void obProtect(bool on); // making sure obEnd() never passes current level
  void flush();
  StringBuffer* swapOutputBuffer(StringBuffer*);
  String getRawPostData() const;
  void setRawPostData(const String& pd);

  /**
   * Request sequences and program execution hooks.
   */
  void registerRequestEventHandler(RequestEventHandler* handler);
  void registerShutdownFunction(const Variant& function, Array arguments,
                                ShutdownType type);
  bool removeShutdownFunction(const Variant& function, ShutdownType type);
  bool hasShutdownFunctions(ShutdownType type);
  void onRequestShutdown();
  void onShutdownPreSend();
  void onShutdownPostSend();

  /**
   * Error handling
   */
  Variant pushUserErrorHandler(const Variant& function, int error_types);
  Variant pushUserExceptionHandler(const Variant& function);
  void popUserErrorHandler();
  void popUserExceptionHandler();
  bool errorNeedsHandling(int errnum,
                          bool callUserHandler,
                          ErrorThrowMode mode);
  bool errorNeedsLogging(int errnum);
  void handleError(const std::string &msg,
                   int errnum,
                   bool callUserHandler,
                   ErrorThrowMode mode,
                   const std::string &prefix,
                   bool skipFrame = false);
  bool callUserErrorHandler(const Exception &e, int errnum,
                            bool swallowExceptions);
  void recordLastError(const Exception &e, int errnum = 0);
  void clearLastError();
  bool onFatalError(const Exception &e); // returns handled
  bool onUnhandledException(Object e);
  ErrorState getErrorState() const;
  void setErrorState(ErrorState);
  String getLastError() const;
  int getLastErrorNumber() const;
  String getErrorPage() const;
  void setErrorPage(const String&);
  String getLastErrorPath() const;
  int getLastErrorLine() const;

  /**
   * Misc. settings
   */
  String getenv(const String& name) const;
  void setenv(const String& name, const String& value);
  void unsetenv(const String& name);
  Array getEnvs() const;

  String getTimeZone() const;
  void setTimeZone(const String&);

  String getDefaultTimeZone() const;
  void setDefaultTimeZone(const String&);

  bool getThrowAllErrors() const;
  void setThrowAllErrors(bool);

  Variant getExitCallback();
  void setExitCallback(Variant);

  void setStreamContext(Resource&);
  Resource& getStreamContext();

  int getPageletTasksStarted() const;
  void incrPageletTasksStarted();

  const VirtualHost* getVirtualHost() const;
  void setVirtualHost(const VirtualHost*);

  const String& getSandboxId() const;
  void setSandboxId(const String&);

  bool hasRequestEventHandlers() const;

private:
  struct OutputBuffer {
    explicit OutputBuffer(Variant&& h, int chunk_sz)
      : oss(8192), handler(std::move(h)), chunk_size(chunk_sz)
    {}
    StringBuffer oss;
    Variant handler;
    int chunk_size;
  };

private:
  // helper functions
  void resetCurrentBuffer();
  void executeFunctions(ShutdownType type);

public:
  void requestInit();
  void requestExit();
  void pushLocalsAndIterators(const Func* f, int nparams = 0);
  void enqueueAPCHandle(APCHandle* handle, size_t size);

  void manageAPCHandle();
  void cleanup();

public:
  const Func* lookupMethodCtx(const Class* cls,
                                        const StringData* methodName,
                                        const Class* pctx,
                                        CallType lookupType,
                                        bool raise = false);
  LookupResult lookupObjMethod(const Func*& f,
                               const Class* cls,
                               const StringData* methodName,
                               const Class* ctx,
                               bool raise = false);
  LookupResult lookupClsMethod(const Func*& f,
                               const Class* cls,
                               const StringData* methodName,
                               ObjectData* this_,
                               const Class* ctx,
                               bool raise = false);
  LookupResult lookupCtorMethod(const Func*& f,
                                const Class* cls,
                                bool raise = false);
  ObjectData* createObject(const Class* cls,
                           const Variant& params,
                           bool init);
  ObjectData* createObject(StringData* clsName,
                           const Variant& params,
                           bool init = true);
  ObjectData* initObject(const Class* cls,
                         const Variant& params,
                         ObjectData* o);
  ObjectData* initObject(StringData* clsName,
                         const Variant& params,
                         ObjectData* o);
  ObjectData* createObjectOnly(StringData* clsName);

  /*
   * Look up a class constant.
   *
   * The returned Cell is guaranteed not to hold a reference counted
   * type.  Raises an error if the class has no constant with that
   * name, or if the class is not defined.
   */
  Cell lookupClsCns(const NamedEntity* ne,
                    const StringData* cls,
                    const StringData* cns);
  Cell lookupClsCns(const StringData* cls,
                    const StringData* cns);

  // Get the next outermost VM frame, even across re-entry
  ActRec* getOuterVMFrame(const ActRec* ar);

  std::string prettyStack(const std::string& prefix) const;
  static void DumpStack();
  static void DumpCurUnit(int skip = 0);
  static void PrintTCCallerInfo();

  ActRec* getStackFrame();
  ObjectData* getThis();
  Class* getContextClass();
  Class* getParentContextClass();
  StringData* getContainingFileName();
  int getLine();
  Array getCallerInfo();
  bool evalUnit(Unit* unit, PC& pc, int funcType);
  void invokeUnit(TypedValue* retval, const Unit* unit);
  Unit* compileEvalString(StringData* code,
                                const char* evalFilename = nullptr);
  StrNR createFunction(const String& args, const String& code);

  // Compiles the passed string and evaluates it in the given frame. Returns
  // false on failure.
  bool evalPHPDebugger(TypedValue* retval, StringData* code, int frame);

  // Evaluates the a unit compiled via compile_string in the given frame.
  // Returns false on failure.
  bool evalPHPDebugger(TypedValue* retval, Unit* unit, int frame);

  void enterDebuggerDummyEnv();
  void exitDebuggerDummyEnv();
  void preventReturnsToTC();
  void preventReturnToTC(ActRec* ar);
  void destructObjects();

  bool isNested() { return m_nesting != 0; }
  void pushVMState(Cell* savedSP);
  void popVMState();

  /**
   * If you call this, you might break some assumption that the JIT made.
   * Ask a JIT expert if your use is ok. The most common use is covered by
   * getPrevFunc so use that if you only want the Func*. That's safe.
   */
  ActRec* getPrevVMStateUNSAFE(const ActRec* fp,
                               Offset* prevPc = nullptr,
                               TypedValue** prevSp = nullptr,
                               bool* fromVMEntry = nullptr);

  const Func* getPrevFunc(const ActRec*);

  VarEnv* getVarEnv(int frame = 0);
  void setVar(StringData* name, const TypedValue* v);
  void bindVar(StringData* name, TypedValue* v);
  Array getLocalDefinedVariables(int frame);
  bool doFCall(ActRec* ar, PC& pc);
  bool doFCallArrayTC(PC pc);
  const Variant& getEvaledArg(const StringData* val,
                              const String& namespacedName);

  enum class CallArrOnInvalidContainer {
    // task #1756122: warning and returning null is what we /should/ always
    // do in call_user_func_array, but some code depends on the broken
    // behavior of casting the list of args to FCallArray to an array.
    CastToArray,
    WarnAndReturnNull,
    WarnAndContinue
  };
  bool doFCallArray(PC& pc, int stkSize, CallArrOnInvalidContainer);
private:
  enum class StackArgsState { // tells prepareFuncEntry how much work to do
    // the stack may contain more arguments than the function expects
    Untrimmed,
    // the stack has already been trimmed of any extra arguments, which
    // have been teleported away into ExtraArgs and/or a variadic param
    Trimmed
  };
  void enterVMAtAsyncFunc(ActRec* enterFnAr, Resumable* resumable,
                          ObjectData* exception);
  void enterVMAtFunc(ActRec* enterFnAr, StackArgsState stk);
  void enterVMAtCurPC();
  void enterVM(ActRec* ar, StackArgsState stackTrimmed,
               Resumable* resumable = nullptr, ObjectData* exception = nullptr);
  void doFPushCuf(PC& pc, bool forward, bool safe);
  template <bool forwarding>
  void pushClsMethodImpl(Class* cls, StringData* name,
                         ObjectData* obj, int numArgs);
  void prepareFuncEntry(ActRec* ar, PC& pc, StackArgsState stk);
  void shuffleMagicArgs(ActRec* ar);
  void shuffleExtraStackArgs(ActRec* ar);
public:
  void syncGdbState();

  enum InvokeFlags {
    InvokeNormal,
    InvokeCuf,
    InvokePseudoMain
  };

  void invokeFunc(TypedValue* retval,
                  const Func* f,
                  const Variant& args_ = init_null_variant,
                  ObjectData* this_ = nullptr,
                  Class* class_ = nullptr,
                  VarEnv* varEnv = nullptr,
                  StringData* invName = nullptr,
                  InvokeFlags flags = InvokeNormal);

  void invokeFunc(TypedValue* retval,
                  const CallCtx& ctx,
                  const Variant& args_,
                  VarEnv* varEnv = nullptr);

  void invokeFuncFew(TypedValue* retval,
                     const Func* f,
                     void* thisOrCls,
                     StringData* invName,
                     int argc,
                     const TypedValue* argv);

  void invokeFuncFew(TypedValue* retval,
                     const Func* f,
                     void* thisOrCls,
                     StringData* invName = nullptr);

  void invokeFuncFew(TypedValue* retval,
                     const CallCtx& ctx,
                     int argc,
                     const TypedValue* argv);

  TypedValue invokeMethod(
    ObjectData* obj,
    const Func* meth,
    InvokeArgs args = InvokeArgs()
  );

  Variant invokeMethodV(
    ObjectData* obj,
    const Func* meth,
    InvokeArgs args = InvokeArgs()
  );

  void resumeAsyncFunc(Resumable* resumable, ObjectData* freeObj,
                       Cell awaitResult);
  void resumeAsyncFuncThrow(Resumable* resumable, ObjectData* freeObj,
                            ObjectData* exception);

  template<typename T>
  using SmartStringIMap = smart::hash_map<
    String,
    T,
    hphp_string_hash,
    hphp_string_isame
  >;

  void dispatch(); // run interpreter normally.
  void dispatchBB(); // exits if a control-flow instruction has been run.

///////////////////////////////////////////////////////////////////////////////
// only fields past here, please.
private:
  // system settings
  Transport* m_transport;
  String m_cwd;

  // output buffering
  StringBuffer* m_sb = nullptr; // current buffer being populated with data
  OutputBuffer* m_out = nullptr; // current OutputBuffer
  int m_remember_chunk = 0; // in case the output buffer is swapped
  smart::list<OutputBuffer> m_buffers; // a stack of output buffers
  bool m_insideOBHandler{false};
  bool m_implicitFlush;
  int m_protectedLevel;
  PFUNC_STDOUT m_stdout;
  void* m_stdoutData;
  size_t m_stdoutBytesWritten;
  String m_rawPostData;

  // request handlers
  smart::vector<RequestEventHandler*> m_requestEventHandlers;
  Array m_shutdowns;

  // error handling
  smart::vector<std::pair<Variant,int>> m_userErrorHandlers;
  smart::vector<Variant> m_userExceptionHandlers;
  ErrorState m_errorState;
  String m_lastError;
  int m_lastErrorNum;
  String m_errorPage;

  // misc settings
  Array m_envs;
  String m_timezone;
  String m_timezoneDefault;
  bool m_throwAllErrors;
  Resource m_streamContext;

  // session backup/restore for RPCRequestHandler
  Array m_shutdownsBackup;
  smart::vector<std::pair<Variant,int>> m_userErrorHandlersBackup;
  smart::vector<Variant> m_userExceptionHandlersBackup;
  Variant m_exitCallback;
  String m_sandboxId; // cache the sandbox id for the request
  int m_pageletTasksStarted;
  const VirtualHost* m_vhost;
public:
  DebuggerSettings debuggerSettings;
  smart::set<ObjectData*> m_liveBCObjs;
private:
  size_t m_apcMemSize{0};
  std::vector<APCHandle*> m_apcHandles; // gets moved to treadmill
public:
  // Although the error handlers may want to access dynamic properties,
  // we cannot *call* the error handlers (or their destructors) while
  // destroying the context, so C++ order of destruction is not an issue.
  smart::hash_map<const ObjectData*,ArrayNoDtor> dynPropTable;
  VarEnv* m_globalVarEnv;
  smart::hash_map<const StringData*,Unit*,string_data_hash,string_data_same>
    m_evaledFiles;
  smart::vector<const StringData*> m_evaledFilesOrder;
  smart::vector<Unit*> m_createdFuncs;
  smart::vector<Fault> m_faults;
  int m_lambdaCounter;
  TinyVector<VMState, 32> m_nestedVMs;
  int m_nesting;
  bool m_dbgNoBreak;
private:
  Array m_evaledArgs;
  String m_lastErrorPath;
  int m_lastErrorLine;
public:
  Variant m_setprofileCallback;
  bool m_executingSetprofileCallback;
  smart::vector<vixl::Simulator*> m_activeSims;
};

///////////////////////////////////////////////////////////////////////////////

template<> void ThreadLocalNoCheck<ExecutionContext>::destroy();

extern DECLARE_THREAD_LOCAL_NO_CHECK(ExecutionContext, g_context);

///////////////////////////////////////////////////////////////////////////////
}

#include "hphp/runtime/base/execution-context-inl.h"

#endif
