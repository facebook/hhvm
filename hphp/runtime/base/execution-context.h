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

#ifndef incl_HPHP_EXECUTION_CONTEXT_H_
#define incl_HPHP_EXECUTION_CONTEXT_H_

#include <list>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "hphp/util/lock.h"
#include "hphp/util/logger.h"
#include "hphp/util/thread-local.h"
#include "hphp/util/tiny-vector.h"
#include "hphp/runtime/base/apc-handle.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/ext/stream/ext_stream.h"
#include "hphp/runtime/server/transport.h"
#include "hphp/runtime/server/virtual-host.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/minstr-state.h"
#include "hphp/runtime/vm/pc-filter.h"

namespace HPHP {
struct RequestEventHandler;
struct EventHook;
struct Resumable;
}

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct VMState {
  PC pc;
  ActRec* fp;
  ActRec* firstAR;
  TypedValue* sp;
  MInstrState mInstrState;
  ActRec* jitCalledFrame;
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

enum class OBFlags {
  None = 0,
  Cleanable = 1,
  Flushable = 2,
  Removable = 4,
  OutputDisabled = 8,
  WriteToStdout = 16,
  Default = 1 | 2 | 4
};

inline OBFlags operator|(const OBFlags& l, const OBFlags& r) {
  return static_cast<OBFlags>(static_cast<int>(l) | static_cast<int>(r));
}

inline OBFlags & operator|=(OBFlags& l, const OBFlags& r) {
  return l = l | r;
}

inline OBFlags operator&(const OBFlags& l, const OBFlags& r) {
  return static_cast<OBFlags>(static_cast<int>(l) & static_cast<int>(r));
}

inline bool any(OBFlags f) { return f != OBFlags::None; }
inline bool operator!(OBFlags f) { return f == OBFlags::None; }

struct VMParserFrame {
  std::string filename;
  int lineNumber;
};

struct DebuggerSettings {
  bool bypassCheck = false;
  bool stackArgs = true;
  int printLevel = -1;
};

struct ThrowAllErrorsSetter {
  ThrowAllErrorsSetter();
  ~ThrowAllErrorsSetter();

private:
  bool m_throwAllErrors;
};

using InvokeArgs = folly::Range<const TypedValue*>;

///////////////////////////////////////////////////////////////////////////////

struct ExecutionContext {
  friend ThrowAllErrorsSetter;

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

  /**
   * Write to the transport, or to stdout if there is no transport.
   */
  void writeTransport(const char* s, int len);

  struct StdoutHook {
    virtual void operator()(const char* s, int len) = 0;
    virtual ~StdoutHook() {};
  };
  void setStdout(StdoutHook*);

  /**
   * Output buffering.
   */
  void obStart(const Variant& handler = uninit_null(),
               int chunk_size = 0,
               OBFlags flags = OBFlags::Default);
  String obCopyContents();
  String obDetachContents();
  int obGetContentLength();
  void obClean(int handler_flag);
  bool obFlush(bool force = false);
  void obFlushAll();
  bool obEnd();
  void obEndAll();
  int obGetLevel();
  String obGetBufferName();
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
  void acceptRequestEventHandlers(bool enable);
  std::size_t registerRequestEventHandler(RequestEventHandler* handler);
  void unregisterRequestEventHandler(RequestEventHandler* handler,
                                     std::size_t index);
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
  void clearUserErrorHandlers();
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

  // Obtain the current queued errors, resetting the queue in the process.
  Array releaseDeferredErrors();

  /**
   * Misc. settings
   */
  String getenv(const String& name) const;
  void setenv(const String& name, const String& value);
  void unsetenv(const String& name);
  Array getEnvs() const;

  String getTimeZone() const;
  void setTimeZone(const String&);

  bool getThrowAllErrors() const;

  Variant getExitCallback();
  void setExitCallback(Variant);

  void setStreamContext(const req::ptr<StreamContext>&);
  const req::ptr<StreamContext>& getStreamContext();

  int getPageletTasksStarted() const;
  void incrPageletTasksStarted();

  const VirtualHost* getVirtualHost() const;
  void setVirtualHost(const VirtualHost*);

  const String& getSandboxId() const;
  void setSandboxId(const String&);

  bool hasRequestEventHandlers() const;

private:
  struct OutputBuffer {
    explicit OutputBuffer(Variant&& h, int chunk_sz, OBFlags flgs)
      : oss(8192), handler(std::move(h)), chunk_size(chunk_sz), flags(flgs)
    {}
    StringBuffer oss;
    Variant handler;
    int chunk_size;
    OBFlags flags;
  };

private:
  // helper functions
  void resetCurrentBuffer();
  void executeFunctions(ShutdownType type);
  void setThrowAllErrors(bool);

public:
  void requestInit();
  void requestExit();
  void enqueueAPCHandle(APCHandle* handle, size_t size);

  void manageAPCHandle();
  void cleanup();

public:
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

  ActRec* getStackFrame();
  ObjectData* getThis();
  Class* getContextClass();
  Class* getParentContextClass();
  StringData* getContainingFileName();
  int getLine();
  Array getCallerInfo();
  bool evalUnit(Unit* unit, PC& pc, int funcType);
  TypedValue invokeUnit(const Unit* unit);
  Unit* compileEvalString(StringData* code,
                                const char* evalFilename = nullptr);
  StrNR createFunction(const String& args, const String& code);

  struct EvaluationResult {
    bool failed;
    Variant result;
    std::string error;
  };

  // Evaluates the given unit in the Nth frame from the current frame (ignoring
  // skip-frames).
  EvaluationResult evalPHPDebugger(StringData* code, int frame);
  EvaluationResult evalPHPDebugger(Unit* unit, int frame);

  void enterDebuggerDummyEnv();
  void exitDebuggerDummyEnv();
  void destructObjects();

  bool isNested() { return m_nesting != 0; }
  void pushVMState(Cell* savedSP);
  void popVMState();

  /*
   * Given a pointer to a VM frame, returns the previous VM frame in the call
   * stack. This function will also pass back by reference the previous PC (if
   * prevPc is non-null) and the previous SP (if prevSp is non-null).
   *
   * If there is no previous VM frame, this function returns NULL and does not
   * set prevPc and prevSp.
   *
   * Inspecting live VM frames other than the current one can be dangerous, so
   * use this function with care. If all you need is the Func that called a
   * particular frame, use getPrevFunc() instead.
   */
  ActRec* getPrevVMState(const ActRec* fp,
                         Offset* prevPc = nullptr,
                         TypedValue** prevSp = nullptr,
                         bool* fromVMEntry = nullptr);
  ActRec* getPrevVMStateSkipFrame(const ActRec* fp,
                                  Offset* prevPc = nullptr,
                                  TypedValue** prevSp = nullptr,
                                  bool* fromVMEntry = nullptr);
  /*
   * Returns the caller of the given frame.
   */
  const Func* getPrevFunc(const ActRec*);

  ActRec* getFrameAtDepth(int frame = 0);
  VarEnv* getOrCreateVarEnv(int frame = 0);
  VarEnv* hasVarEnv(int frame = 0);
  void setVar(StringData* name, const TypedValue* v);
  void bindVar(StringData* name, TypedValue* v);
  Array getLocalDefinedVariables(int frame);
  const Variant& getEvaledArg(const StringData* val,
                              const String& namespacedName,
                              const Unit* funcUnit);

private:
  template <bool forwarding>
  void pushClsMethodImpl(Class* cls, StringData* name,
                         ObjectData* obj, int numArgs);
public:
  void syncGdbState();

  enum InvokeFlags {
    InvokeNormal,
    InvokeCuf,
    InvokePseudoMain
  };

  TypedValue invokeFunc(const Func* f,
                        const Variant& args_ = init_null_variant,
                        ObjectData* this_ = nullptr,
                        Class* class_ = nullptr,
                        VarEnv* varEnv = nullptr,
                        StringData* invName = nullptr,
                        InvokeFlags flags = InvokeNormal,
                        bool useWeakTypes = false);

  TypedValue invokeFunc(const CallCtx& ctx,
                        const Variant& args_,
                        VarEnv* varEnv = nullptr);

  TypedValue invokeFuncFew(const Func* f,
                           void* thisOrCls,
                           StringData* invName,
                           int argc,
                           const TypedValue* argv,
                           bool useWeakTypes = false);

  TypedValue invokeFuncFew(const Func* f,
                           void* thisOrCls,
                           StringData* invName = nullptr);

  TypedValue invokeFuncFew(const CallCtx& ctx,
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

  bool setHeaderCallback(const Variant& callback);

  template<class Fn> void sweepDynPropTable(Fn);

private:
  template<class FStackCheck, class FInitArgs, class FEnterVM>
  TypedValue invokeFuncImpl(const Func* f,
                            ObjectData* thiz, Class* cls, uint32_t argc,
                            StringData* invName, bool useWeakTypes,
                            FStackCheck doStackCheck,
                            FInitArgs doInitArgs,
                            FEnterVM doEnterVM);

  struct ExcLoggerHook final : LoggerHook {
    explicit ExcLoggerHook(ExecutionContext& ec) : ec(ec) {}
    void operator()(const char* header, const char* msg, const char* ending)
         override;
    ExecutionContext& ec;
  };

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
  req::list<OutputBuffer> m_buffers; // a stack of output buffers
  bool m_insideOBHandler{false};
  bool m_implicitFlush;
  int m_protectedLevel;
  StdoutHook* m_stdout;
  size_t m_stdoutBytesWritten;
  String m_rawPostData;

  // request handlers
  req::vector<RequestEventHandler*> m_requestEventHandlers;
  Array m_shutdowns;
  bool m_acceptRequestEventHandlers;

  // error handling
  req::vector<std::pair<Variant,int>> m_userErrorHandlers;
  req::vector<Variant> m_userExceptionHandlers;
  ErrorState m_errorState;
  String m_lastError;
  int m_lastErrorNum;
  String m_errorPage;
  Array m_deferredErrors;

  // misc settings
  Array m_envs;
  String m_timezone;
  bool m_throwAllErrors;
  req::ptr<StreamContext> m_streamContext;

  // session backup/restore for RPCRequestHandler
  Array m_shutdownsBackup;
  req::vector<std::pair<Variant,int>> m_userErrorHandlersBackup;
  req::vector<Variant> m_userExceptionHandlersBackup;
  Variant m_exitCallback;
  String m_sandboxId; // cache the sandbox id for the request
  int m_pageletTasksStarted;
  const VirtualHost* m_vhost;
public:
  DebuggerSettings debuggerSettings;
  req::set<ObjectData*> m_liveBCObjs; // objects with destructors
private:
  size_t m_apcMemSize{0};
  std::vector<APCHandle*> m_apcHandles; // gets moved to treadmill
public:
  // Although the error handlers may want to access dynamic properties,
  // we cannot *call* the error handlers (or their destructors) while
  // destroying the context, so C++ order of destruction is not an issue.
  req::hash_map<const ObjectData*,ArrayNoDtor> dynPropTable;
  TYPE_SCAN_IGNORE_FIELD(dynPropTable);
  VarEnv* m_globalVarEnv;
  struct FileInfo {
    Unit* unit;
    time_t ts_sec; // timestamp seconds
    unsigned long ts_nsec; // timestamp nanoseconds (or 0 if ns not supported)
  };
  req::hash_map<const StringData*, FileInfo, string_data_hash, string_data_same>
    m_evaledFiles;
  req::vector<const StringData*> m_evaledFilesOrder;
  req::vector<Unit*> m_createdFuncs;
  req::vector<Fault> m_faults;
  int m_lambdaCounter;
  req::TinyVector<VMState, 32> m_nestedVMs;
  int m_nesting;
  bool m_dbgNoBreak;
  bool m_unwindingCppException;
private:
  Array m_evaledArgs;
  String m_lastErrorPath;
  int m_lastErrorLine;
public:
  Variant m_setprofileCallback;
  Variant m_memThresholdCallback;
  uint64_t m_setprofileFlags;
  bool m_executingSetprofileCallback;
public:
  Cell m_headerCallback;
  bool m_headerCallbackDone{false}; // used to prevent infinite loops
private:
  ExcLoggerHook m_logger_hook;
};

///////////////////////////////////////////////////////////////////////////////

// MSVC doesn't instantiate this, causing an undefined symbol at link time
// if the template<> is present, but other compilers require it.
#ifndef _MSC_VER
template<>
#endif
void ThreadLocalNoCheck<ExecutionContext>::destroy();

extern DECLARE_THREAD_LOCAL_NO_CHECK(ExecutionContext, g_context);

///////////////////////////////////////////////////////////////////////////////
}

#include "hphp/runtime/base/execution-context-inl.h"

#endif
