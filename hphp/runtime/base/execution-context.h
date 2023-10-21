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

#include "hphp/runtime/base/apc-handle.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/recorder.h"
#include "hphp/runtime/base/req-list.h"
#include "hphp/runtime/base/req-tiny-vector.h"
#include "hphp/runtime/base/req-vector.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/vanilla-dict.h"
#include "hphp/runtime/ext/stream/ext_stream.h"
#include "hphp/runtime/server/transport.h"
#include "hphp/runtime/server/virtual-host.h"
#include "hphp/runtime/vm/coeffects.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/minstr-state.h"
#include "hphp/runtime/vm/pc-filter.h"

#include "hphp/util/lock.h"
#include "hphp/util/logger.h"
#include "hphp/util/optional.h"
#include "hphp/util/rds-local.h"
#include "hphp/util/thread-local.h"

#include <list>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace HPHP {
struct RequestEventHandler;
struct EventHook;
struct Resumable;
namespace stream_transport {
struct StreamTransport;
}
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
  jit::TCA jitReturnAddr;
  Either<ObjectData*, Exception*> exn;
  bool unwinderSideEnter;
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
  LowStringPtr filename;
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

enum class FileLoadFlags {
  kDup,
  kHitMem,
  kWaited,
  kHitDisk,
  kCompiled,
  kEvicted
};

using InvokeArgs = folly::Range<const TypedValue*>;

///////////////////////////////////////////////////////////////////////////////

struct ExecutionContext {
  friend ThrowAllErrorsSetter;

  enum ShutdownType {
    ShutDown = 0,
    PostSend = 1,
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
  std::shared_ptr<stream_transport::StreamTransport> getServerStreamTransport() const;
  void setRequestTrace(rqtrace::Trace*);
  std::string getRequestUrl(size_t szLimit = std::string::npos);
  String getMimeType() const;
  void setContentType(const String& mimetype, const String& charset);
  String getCwd() const;
  void setCwd(const String&);
  rqtrace::Trace* getRequestTrace();

  /**
   * Write to output.
   */
  void write(const String&);
  void write(const char* s, int len);
  void write(const char*);

  void writeStdout(const char* s, int len, bool skipHooks = false);
  size_t getStdoutBytesWritten() const;

  /**
   * Write to the transport, or to stdout if there is no transport.
   */
  void writeTransport(const char* s, int len);

  struct StdoutHook {
    virtual void operator()(const char* s, int len) = 0;
    virtual ~StdoutHook() {};
  };
  void addStdoutHook(StdoutHook*);
  bool removeStdoutHook(StdoutHook*);
  std::size_t numStdoutHooks() const;

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
  void registerShutdownFunction(const Variant& function, ShutdownType type);
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
                   const std::string& prefix,
                   bool skipFrame = false);
  bool callUserErrorHandler(const Exception& e, int errnum,
                            bool swallowExceptions);
  void recordLastError(const Exception& e, int errnum = 0);
  void clearLastError();
  bool onFatalError(const Exception& e); // returns handled
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

  String getTimezone() const;
  void setTimezone(const String&);

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

  const PackageInfo& getPackageInfo() const;

  const RepoOptions& getRepoOptionsForCurrentFrame() const;
  const RepoOptions& getRepoOptionsForFrame(int frame) const;

  const RepoOptions* getRepoOptionsForRequest() const;

  // When a file is loaded inside of a request context we perform a consistency
  // check to ensure that all files loaded within the request use the same
  // options.
  void onLoadWithOptions(const char* f, const RepoOptions& options);

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
  void enqueueAPCDeferredExpire(const String&);
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
   * The returned TypedValue is guaranteed not to hold a reference counted
   * type.  Raises an error if the class has no constant with that
   * name, or if the class is not defined.
   */
  TypedValue lookupClsCns(const NamedType*,
                    const StringData* cls,
                    const StringData* cns);
  TypedValue lookupClsCns(const StringData* cls,
                    const StringData* cns);

  // Get the next outermost VM frame, even across re-entry
  ActRec* getOuterVMFrame(const ActRec* ar);

  ActRec* getStackFrame();
  ObjectData* getThis();
  StringData* getContainingFileName();
  int getLine();
  TypedValue invokeUnit(const Unit* unit, bool callByHPHPInvoke = false);

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

  Array& getDebuggerEnv() { return m_debuggerEnv; }

  bool isNested() { return m_nesting != 0; }
  void pushVMState(TypedValue* savedSP);
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
                         bool* fromVMEntry = nullptr,
                         jit::TCA* jitReturnAddr = nullptr);
  ActRec* getPrevVMStateSkipFrame(const ActRec* fp,
                                  Offset* prevPc = nullptr,
                                  TypedValue** prevSp = nullptr,
                                  bool* fromVMEntry = nullptr);
  /*
   * Returns the caller of the given frame.
   */
  const Func* getPrevFunc(const ActRec*);

  /*
   * Returns the call frame at the specified depth, intended only
   * for use by the debugger. Use in other contexts may not be safe.
   */
  ActRec* getFrameAtDepthForDebuggerUnsafe(int frame = 0) const;
  Array getLocalDefinedVariablesDebugger(int frame);
  Variant getEvaledArg(const StringData* val,
                       const String& namespacedName,
                       const Unit* funcUnit);

private:
  template <bool forwarding>
  void pushClsMethodImpl(Class* cls, StringData* name,
                         ObjectData* obj, int numArgs);
public:
  void syncGdbState();

  using ThisOrClass = Either<ObjectData*, Class*>;

  TypedValue invokeFunc(const Func* f,
                        const Variant& args_ = init_null_variant,
                        ObjectData* this_ = nullptr,
                        Class* class_ = nullptr,
                        RuntimeCoeffects providedCoeffects =
                          RuntimeCoeffects::fixme(),
                        bool dynamic = true,
                        bool checkRefAnnot = false,
                        bool allowDynCallNoPointer = false,
                        bool readonlyReturn = false,
                        Array&& reifiedGenerics = Array());

  TypedValue invokeFunc(const CallCtx& ctx,
                        const Variant& args_,
                        RuntimeCoeffects providedCoeffects);

  TypedValue invokeFuncFew(const Func* f,
                           ThisOrClass thisOrCls,
                           uint32_t numArgs,
                           const TypedValue* argv,
                           RuntimeCoeffects providedCoeffects,
                           bool dynamic = true,
                           bool allowDynCallNoPointer = false);

  TypedValue invokeFuncFew(const Func* f,
                           ThisOrClass thisOrCls,
                           RuntimeCoeffects providedCoeffects);

  TypedValue invokeFuncFew(const CallCtx& ctx,
                           uint32_t numArgs,
                           const TypedValue* argv,
                           RuntimeCoeffects providedCoeffects);

  TypedValue invokeMethod(
    ObjectData* obj,
    const Func* meth,
    InvokeArgs args,
    RuntimeCoeffects providedCoeffects
  );

  Variant invokeMethodV(
    ObjectData* obj,
    const Func* meth,
    InvokeArgs args,
    RuntimeCoeffects providedCoeffects
  );

  void resumeAsyncFunc(Resumable* resumable, ObjectData* freeObj,
                       TypedValue awaitResult);
  void resumeAsyncFuncThrow(Resumable* resumable, ObjectData* freeObj,
                            ObjectData* exception);

  bool setHeaderCallback(const Variant& callback);

  template<class Fn> void sweepDynPropTable(Fn);

private:
  TypedValue invokeFuncImpl(const Func* f, ObjectData* thiz, Class* cls,
                            uint32_t numArgsInclUnpack,
                            RuntimeCoeffects providedCoeffects,
                            bool hasGenerics,
                            bool dynamic, bool allowDynCallNoPointer);

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

  std::unordered_set<StdoutHook*> m_stdoutHooks;
  size_t m_stdoutBytesWritten;
  String m_rawPostData;

  // request handlers
  req::vector<RequestEventHandler*> m_requestEventHandlers;
  std::array<Array, 2> m_shutdowns;
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
  std::array<Array, 2> m_shutdownsBackup;
  req::vector<std::pair<Variant,int>> m_userErrorHandlersBackup;
  req::vector<Variant> m_userExceptionHandlersBackup;
  Variant m_exitCallback;
  String m_sandboxId; // cache the sandbox id for the request
  int m_pageletTasksStarted;
  const VirtualHost* m_vhost;
public:
  DebuggerSettings debuggerSettings;
private:
  size_t m_apcMemSize{0};
  std::vector<APCHandle*> m_apcHandles; // gets moved to treadmill
  req::vector<StringData*> m_apcDeferredExpire;
public:
  // Although the error handlers may want to access dynamic properties,
  // we cannot *call* the error handlers (or their destructors) while
  // destroying the context, so C++ order of destruction is not an issue.
  req::fast_map<const ObjectData*,ArrayNoDtor> dynPropTable;
  TYPE_SCAN_IGNORE_FIELD(dynPropTable);
  NameValueTable* m_globalNVTable;
  struct FileInfo {
    Unit* unit;
    time_t ts_sec; // timestamp seconds
    unsigned long ts_nsec; // timestamp nanoseconds (or 0 if ns not supported)
    FileLoadFlags flags;
  };
  req::fast_map<const StringData*, FileInfo, string_data_hash, string_data_same>
    m_evaledFiles;
  req::vector<const StringData*> m_evaledFilesOrder;
  req::fast_set<const Unit*> m_loadedUnits; // Used only by debuggers
  req::fast_set<Unit*> m_touchedUnits;
  Array m_visitedFiles;
  int m_lambdaCounter;
  using VMStateVec = req::TinyVector<VMState, 32>;
  VMStateVec m_nestedVMs;
  TYPE_SCAN_IGNORE_FIELD(m_nestedVMs); // handled explicitly in heap-scan.h
  int m_nesting;
  bool m_dbgNoBreak;
  // Once we've started inline-interp, set it to BLOCK to throw on re-entry.
  enum class InlineInterpState { NONE, START, BLOCK };
  InlineInterpState m_inlineInterpState;

  bool doingInlineInterp() const {
    return m_inlineInterpState != InlineInterpState::NONE;
  }

private:
  Array m_evaledArgs;
  String m_lastErrorPath;
  int m_lastErrorLine;
public:
  Variant m_setprofileCallback;
  Variant m_memThresholdCallback;
  Variant m_timeThresholdCallback;
  uint64_t m_setprofileFlags;
  bool m_executingSetprofileCallback;
  req::fast_set<String,
                hphp_string_hash, hphp_string_isame> m_setprofileFunctions;
public:
  enum class InternalEventHook: uint8_t {
    Call = 0,
    Return = 1,
    Resume = 2,
    Suspend = 3,
    Unwind = 4,
  };
  using InternalEventHookCallbackType = void(*)(const ActRec*,
                                                InternalEventHook);
  InternalEventHookCallbackType m_internalEventHookCallback{nullptr};

public:
  TypedValue m_headerCallback;
  bool m_headerCallbackDone{false}; // used to prevent infinite loops
private:
  ExcLoggerHook m_logger_hook;
  rqtrace::Trace* m_requestTrace{nullptr};

  Optional<RepoOptions> m_requestOptions;

  Array m_debuggerEnv; // variables read/written in the REPL
public:
  VMParserFrame* m_parserFrame{nullptr};

  Optional<struct timespec> m_requestStartForTearing;

  // When logging request tearing we store a fast map of deps to the paths to
  // units that depend on them and the SHA-1s that they observed.
  req::fast_map<
    const StringData*,
    req::vector<std::pair<std::string, SHA1>>
  > m_loadedRdepMap;

  Optional<Recorder> m_recorder;
};

///////////////////////////////////////////////////////////////////////////////

using GContextType = rds::local::AliasedRDSLocal<ExecutionContext,
      rds::local::Initialize::Explicitly,
      &rds::local::detail::HotRDSLocals::g_context>;

namespace rds::local {
template<>
void GContextType::Base::destroy();
}

// Use AliasedRDSLocal for the ExecutionContext since it is accessed so
// frequently, and AliasedRDSlocal may save up to 1 instruction and 1 load
// per access.
extern GContextType g_context;

///////////////////////////////////////////////////////////////////////////////
}

#include "hphp/runtime/base/execution-context-inl.h"
