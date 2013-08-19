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

#ifndef incl_HPHP_EXECUTION_CONTEXT_H_
#define incl_HPHP_EXECUTION_CONTEXT_H_

#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/server/transport.h"
#include "hphp/runtime/base/debuggable.h"
#include "hphp/runtime/server/virtual-host.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/hphp-array.h"
#include "hphp/runtime/vm/funcdict.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/util/base.h"
#include "hphp/util/lock.h"
#include "hphp/util/thread_local.h"
#include <setjmp.h>

#define PHP_OUTPUT_HANDLER_START  (1<<0)
#define PHP_OUTPUT_HANDLER_CONT   (1<<1)
#define PHP_OUTPUT_HANDLER_END    (1<<2)

namespace HPHP {
class c_Continuation;
namespace Eval {
class PhpFile;
}

class EventHook;
namespace Transl {
class Translator;
}
class PCFilter;

///////////////////////////////////////////////////////////////////////////////

typedef hphp_hash_map<StringData*, HPHP::Eval::PhpFile*, string_data_hash,
                      string_data_same> EvaledFilesMap;

/**
 * Mainly designed for extensions to perform initialization and shutdown
 * sequences at request scope.
 */
class RequestEventHandler {
public:
  RequestEventHandler() : m_inited(false) {}
  virtual ~RequestEventHandler() {}

  virtual void requestInit() = 0;
  virtual void requestShutdown() = 0;

  void setInited(bool inited) { m_inited = inited;}
  bool getInited() const { return m_inited;}

  // Priority of request shutdown call. Lower priority value means
  // requestShutdown is called earlier than higher priority values.
  virtual int priority() const { return 0;}

protected:
  bool m_inited;
};

struct VMState {
  PC pc;
  ActRec* fp;
  ActRec* firstAR;
  TypedValue *sp;
};

class MethodInfoVM : public ClassInfo::MethodInfo,
                     public AtomicCountable {
 public:
  ~MethodInfoVM();
  void atomicRelease() { delete this; }
};

class ClassInfoVM : public ClassInfo,
                    public AtomicCountable {
 public:
  ~ClassInfoVM();
  void atomicRelease() { delete this; }
  virtual CStrRef getParentClass() const { return m_parentClass; }

  const InterfaceSet  &getInterfaces()      const { return m_interfaces;}
  const InterfaceVec  &getInterfacesVec()   const { return m_interfacesVec;}
  const MethodMap     &getMethods()         const { return m_methods;}
  const MethodVec     &getMethodsVec()      const { return m_methodsVec;}
  const PropertyMap   &getProperties()      const { return m_properties;}
  const PropertyVec   &getPropertiesVec()   const { return m_propertiesVec;}
  const ConstantMap   &getConstants()       const { return m_constants;}
  const ConstantVec   &getConstantsVec()    const { return m_constantsVec;}
  const UserAttributeVec &getUserAttributeVec() const { return m_userAttrVec;}
  const TraitSet      &getTraits()          const { return m_traits;}
  const TraitVec      &getTraitsVec()       const { return m_traitsVec;}
  const TraitAliasVec &getTraitAliasesVec() const { return m_traitAliasesVec;}

 private:
  String        m_parentClass;
  InterfaceSet  m_interfaces;      // all interfaces
  InterfaceVec  m_interfacesVec;   // all interfaces
  TraitSet      m_traits;          // all used traits
  TraitVec      m_traitsVec;       // all used traits
  TraitAliasVec m_traitAliasesVec; // all trait aliases
  MethodMap     m_methods;         // all methods
  MethodVec     m_methodsVec;      // in source order
  PropertyMap   m_properties;      // all properties
  PropertyVec   m_propertiesVec;   // in source order
  ConstantMap   m_constants;       // all constants
  ConstantVec   m_constantsVec;    // in source order
  UserAttributeVec m_userAttrVec;

 public:
  friend class HPHP::Class;
};

namespace MethodLookup {
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
}

enum InclOpFlags {
  InclOpDefault = 0,
  InclOpFatal = 1,
  InclOpOnce = 2,
  InclOpDocRoot = 8,
  InclOpRelative = 16,
};

inline InclOpFlags
operator|(const InclOpFlags &l, const InclOpFlags &r) {
  return InclOpFlags(int(l) | int(r));
}

inline InclOpFlags
operator&(const InclOpFlags &l, const InclOpFlags &r) {
  return InclOpFlags(int(l) & int(r));
}

struct VMParserFrame {
  std::string filename;
  int lineNumber;
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Put all global variables here so we can gather them into one thread-local
 * variable for easy access.
 */
class BaseExecutionContext : public IDebuggable {
public:
  // These members are declared first for performance reasons: they
  // are accessed from within the TC and having their offset fit
  // within a single byte makes the generated code slightly smaller
  // and faster.
  Stack m_stack;
  ActRec* m_fp;
  PC m_pc;
  int64_t m_currentThreadIdx;
public:
  enum ShutdownType {
    ShutDown,
    PostSend,
    CleanUp,

    ShutdownTypeCount
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
  BaseExecutionContext();
  ~BaseExecutionContext();

  // For RPCRequestHandler
  void backupSession();
  void restoreSession();

  // implementing IDebuggable
  virtual void debuggerInfo(InfoVec &info);

  /**
   * System settings.
   */
  Transport *getTransport() { return m_transport;}
  void setTransport(Transport *transport) { m_transport = transport;}
  std::string getRequestUrl(size_t szLimit = std::string::npos);
  String getMimeType() const;
  void setContentType(CStrRef mimetype, CStrRef charset);
  int64_t getRequestMemoryMaxBytes() const { return m_maxMemory; }
  void setRequestMemoryMaxBytes(int64_t max);
  String getCwd() const { return m_cwd;}
  void setCwd(CStrRef cwd) { m_cwd = cwd;}

  /**
   * Write to output.
   */
  void write(CStrRef s);
  void write(const char *s, int len);
  void write(const char *s) { write(s, strlen(s));}
  void writeStdout(const char *s, int len);

  typedef void (*PFUNC_STDOUT)(const char *s, int len, void *data);
  void setStdout(PFUNC_STDOUT func, void *data);

  /**
   * Output buffering.
   */
  void obStart(CVarRef handler = uninit_null());
  String obCopyContents();
  String obDetachContents();
  int obGetContentLength();
  void obClean();
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
  StringBuffer *swapOutputBuffer(StringBuffer *sb) {
    StringBuffer *current = m_out;
    m_out = sb;
    return current;
  }

  /**
   * Request sequences and program execution hooks.
   */
  void registerRequestEventHandler(RequestEventHandler *handler);
  void registerShutdownFunction(CVarRef function, Array arguments,
                                ShutdownType type);
  void onRequestShutdown();
  void onShutdownPreSend();
  void onShutdownPostSend();

  /**
   * Error handling
   */
  Variant pushUserErrorHandler(CVarRef function, int error_types);
  Variant pushUserExceptionHandler(CVarRef function);
  void popUserErrorHandler();
  void popUserExceptionHandler();
  bool errorNeedsHandling(int errnum,
                          bool callUserHandler,
                          ErrorThrowMode mode);
  void handleError(const std::string &msg,
                   int errnum,
                   bool callUserHandler,
                   ErrorThrowMode mode,
                   const std::string &prefix,
                   bool skipFrame = false);
  bool callUserErrorHandler(const Exception &e, int errnum,
                            bool swallowExceptions);
  void recordLastError(const Exception &e, int errnum = 0);
  bool onFatalError(const Exception &e); // returns handled
  bool onUnhandledException(Object e);
  ErrorState getErrorState() const { return m_errorState;}
  void setErrorState(ErrorState state) { m_errorState = state;}
  String getLastError() const { return m_lastError;}
  int getLastErrorNumber() const { return m_lastErrorNum;}
  int getErrorReportingLevel() const { return m_errorReportingLevel;}
  void setErrorReportingLevel(int level) { m_errorReportingLevel = level;}
  String getErrorPage() const { return m_errorPage;}
  void setErrorPage(CStrRef page) { m_errorPage = (std::string) page; }
  bool getLogErrors() const { return m_logErrors;}
  void setLogErrors(bool on);
  String getErrorLog() const { return m_errorLog;}
  void setErrorLog(CStrRef filename);

  /**
   * Misc. settings
   */
  String getenv(CStrRef name) const;
  void setenv(CStrRef name, CStrRef value);
  Array getEnvs() const { return m_envs; }

  String getTimeZone() const { return m_timezone;}
  void setTimeZone(CStrRef timezone) { m_timezone = timezone;}
  String getDefaultTimeZone() const { return m_timezoneDefault;}
  String getArgSeparatorOutput() const {
    if (m_argSeparatorOutput.isNull()) return s_amp;
    return m_argSeparatorOutput;
  }
  void setArgSeparatorOutput(CStrRef s) { m_argSeparatorOutput = s;}
  void setThrowAllErrors(bool f) { m_throwAllErrors = f; }
  bool getThrowAllErrors() const { return m_throwAllErrors; }
  void setExitCallback(Variant f) { m_exitCallback = f; }
  Variant getExitCallback() { return m_exitCallback; }

  void setIncludePath(CStrRef path);
  String getIncludePath() const;
  Array getIncludePathArray() const { return m_include_paths; }
  const VirtualHost *getVirtualHost() const { return m_vhost; }
  void setVirtualHost(const VirtualHost *vhost) { m_vhost = vhost; }

  CStrRef getSandboxId() const { return m_sandboxId; }
  void setSandboxId(CStrRef sandboxId) { m_sandboxId = sandboxId; }

  DECLARE_DBG_SETTING_ACCESSORS

private:
  class OutputBuffer {
  public:
    OutputBuffer() : oss(8192) {}
    StringBuffer oss;
    Variant handler;
  };

private:
  static const StaticString s_amp;
  // system settings
  Transport *m_transport;
  int64_t m_maxMemory;
  String m_cwd;

  // output buffering
  StringBuffer *m_out;                // current output buffer
  std::list<OutputBuffer*> m_buffers; // a stack of output buffers
  bool m_implicitFlush;
  int m_protectedLevel;
  PFUNC_STDOUT m_stdout;
  void *m_stdoutData;

  // request handlers
  std::set<RequestEventHandler*> m_requestEventHandlerSet;
  std::vector<RequestEventHandler*> m_requestEventHandlers;
  Array m_shutdowns;

  // error handling
  std::vector<std::pair<Variant,int> > m_userErrorHandlers;
  std::vector<Variant> m_userExceptionHandlers;
  ErrorState m_errorState;
  int m_errorReportingLevel;
  String m_lastError;
  int m_lastErrorNum;
  std::string m_errorPage;
  bool m_logErrors;
  String m_errorLog;

  // misc settings
  Array m_envs;
  String m_timezone;
  String m_timezoneDefault;
  String m_argSeparatorOutput;
  bool m_throwAllErrors;

  // session backup/restore for RPCRequestHandler
  Array m_shutdownsBackup;
  std::vector<std::pair<Variant,int> > m_userErrorHandlersBackup;
  std::vector<Variant> m_userExceptionHandlersBackup;

  Variant m_exitCallback;

  // include_path configuration option
  Array m_include_paths;

  // cache the sandbox id for the request
  String m_sandboxId;

  const VirtualHost *m_vhost;
  // helper functions
  void resetCurrentBuffer();
  void executeFunctions(CArrRef funcs);

  DECLARE_DBG_SETTING
};

class VMExecutionContext : public BaseExecutionContext {
public:
  VMExecutionContext();
  ~VMExecutionContext();

  typedef std::set<ObjectData*> LiveObjSet;
  LiveObjSet m_liveBCObjs;

  // pcre ini_settings
  long m_preg_backtrace_limit;
  long m_preg_recursion_limit;

public:
  void requestInit();
  void requestExit();

  static void getElem(TypedValue* base, TypedValue* key, TypedValue* dest);
  static c_Continuation* createContFunc(const Func* origFunc,
                                        const Func* genFunc);
  static c_Continuation* createContMeth(const Func* origFunc,
                                        const Func* genFunc,
                                        void* objOrCls);
  static void fillContinuationVars(
    ActRec* origFp, const Func* origFunc, ActRec* genFp, const Func* genFunc);
  void pushLocalsAndIterators(const HPHP::Func* f, int nparams = 0);

private:
  enum class VectorLeaveCode {
    ConsumeAll,
    LeaveLast
  };
  template <bool setMember, bool warn, bool define, bool unset, bool reffy,
            unsigned mdepth, VectorLeaveCode mleave, bool saveResult>
  bool memberHelperPre(PC& pc, unsigned& ndiscard, TypedValue*& base,
                       TypedValue& tvScratch,
                       TypedValue& tvLiteral,
                       TypedValue& tvRef, TypedValue& tvRef2,
                       MemberCode& mcode, TypedValue*& curMember);
  template <bool warn, bool saveResult, VectorLeaveCode mleave>
  void getHelperPre(PC& pc, unsigned& ndiscard,
                    TypedValue*& base, TypedValue& tvScratch,
                    TypedValue& tvLiteral,
                    TypedValue& tvRef, TypedValue& tvRef2,
                    MemberCode& mcode, TypedValue*& curMember);
  template <bool saveResult>
  void getHelperPost(unsigned ndiscard, TypedValue*& tvRet,
                     TypedValue& tvScratch, Variant& tvRef,
                     Variant& tvRef2);
  void getHelper(PC& pc, unsigned& ndiscard, TypedValue*& tvRet,
                 TypedValue*& base, TypedValue& tvScratch,
                 TypedValue& tvLiteral,
                 Variant& tvRef, Variant& tvRef2,
                 MemberCode& mcode, TypedValue*& curMember);

  template <bool warn, bool define, bool unset, bool reffy, unsigned mdepth,
            VectorLeaveCode mleave>
  bool setHelperPre(PC& pc, unsigned& ndiscard, TypedValue*& base,
                    TypedValue& tvScratch,
                    TypedValue& tvLiteral,
                    TypedValue& tvRef, TypedValue& tvRef2,
                    MemberCode& mcode, TypedValue*& curMember);
  template <unsigned mdepth>
  void setHelperPost(unsigned ndiscard, Variant& tvRef,
                     Variant& tvRef2);
  template <bool isEmpty>
  void isSetEmptyM(PC& pc);

  template<class Op> void implCellBinOp(PC&, Op op);
  template<class Op> void implCellBinOpBool(PC&, Op op);
  bool cellInstanceOf(TypedValue* c, const HPHP::NamedEntity* s);
  bool initIterator(PC& pc, PC& origPc, Iter* it,
                    Offset offset, Cell* c1);
  bool initIteratorM(PC& pc, PC& origPc, Iter* it,
                     Offset offset, Ref* r1, TypedValue* val, TypedValue* key);
  void jmpSurpriseCheck(Offset o);
  template<Op op> void jmpOpImpl(PC& pc);
  template<class Op> void roundOpImpl(Op op);
#define O(name, imm, pusph, pop, flags)                                       \
  void iop##name(PC& pc);
OPCODES
#undef O

  void classExistsImpl(PC& pc, Attr typeAttr);
  void fPushObjMethodImpl(
      Class* cls, StringData* name, ObjectData* obj, int numArgs);
  ActRec* fPushFuncImpl(const Func* func, int numArgs);

public:
  typedef hphp_hash_map<const StringData*, ClassInfo::ConstantInfo*,
                        string_data_hash, string_data_same> ConstInfoMap;
  ConstInfoMap m_constInfo;
  typedef hphp_hash_map<const HPHP::Class*, HphpArray*,
                        pointer_hash<HPHP::Class> > ClsCnsDataMap;
  ClsCnsDataMap m_clsCnsData;
  typedef hphp_hash_map<const HPHP::Class*, HPHP::Class::PropInitVec*,
                        pointer_hash<HPHP::Class> > PropDataMap;
  PropDataMap m_propData;
  typedef hphp_hash_map<const HPHP::Class*, HphpArray*,
                        pointer_hash<HPHP::Class> > SPropDataMap;
  SPropDataMap m_sPropData;
  typedef hphp_hash_map<const HPHP::Func*, HphpArray*,
                        pointer_hash<HPHP::Func> > FuncStaticCtxMap;
  FuncStaticCtxMap m_funcStaticCtx;

  const HPHP::Func* lookupMethodCtx(const HPHP::Class* cls,
                                        const StringData* methodName,
                                        HPHP::Class* pctx,
                                        MethodLookup::CallType lookupType,
                                        bool raise = false);
  MethodLookup::LookupResult lookupObjMethod(const HPHP::Func*& f,
                                             const HPHP::Class* cls,
                                             const StringData* methodName,
                                             bool raise = false);
  MethodLookup::LookupResult lookupClsMethod(const HPHP::Func*& f,
                                             const HPHP::Class* cls,
                                             const StringData* methodName,
                                             ObjectData* this_,
                                             ActRec* vmfp,
                                             bool raise = false);
  MethodLookup::LookupResult lookupCtorMethod(const HPHP::Func*& f,
                                              const HPHP::Class* cls,
                                              bool raise = false);
  ObjectData* createObject(StringData* clsName,
                           CArrRef params,
                           bool init = true);
  ObjectData* createObjectOnly(StringData* clsName);

  HphpArray* getFuncStaticCtx(const HPHP::Func* f) {
    FuncStaticCtxMap::iterator it = m_funcStaticCtx.find(f);
    if (it != m_funcStaticCtx.end()) {
      return it->second;
    }

    auto array = ArrayData::Make(f->numStaticLocals());
    array->incRefCount();
    return m_funcStaticCtx[f] = array;
  }

  HphpArray* getClsCnsData(const HPHP::Class* class_) const {
    ClsCnsDataMap::const_iterator it = m_clsCnsData.find(class_);
    if (it == m_clsCnsData.end()) {
      return nullptr;
    }
    return it->second;
  }
  void setClsCnsData(const HPHP::Class* class_, HphpArray* clsCnsData) {
    assert(getClsCnsData(class_) == nullptr);
    m_clsCnsData[class_] = clsCnsData;
  }

  Cell* lookupClsCns(const HPHP::NamedEntity* ne,
                     const StringData* cls,
                     const StringData* cns);

  Cell* lookupClsCns(const StringData* cls,
                     const StringData* cns);

  // Get the next outermost VM frame, even accross re-entry
  ActRec* getOuterVMFrame(const ActRec* ar);

  std::string prettyStack(const std::string& prefix) const;
  static void DumpStack();
  static void DumpCurUnit(int skip = 0);
  static void PrintTCCallerInfo();

  VarEnv* m_globalVarEnv;

  HPHP::RenamedFuncDict m_renamedFuncs;
  EvaledFilesMap m_evaledFiles;
  typedef std::vector<HPHP::Unit*> EvaledUnitsVec;
  EvaledUnitsVec m_createdFuncs;

  /*
   * Accessors for VMExecutionContext state that check safety wrt
   * whether these values may be stale due to TC.  Asserts in these
   * usually mean the need for a VMRegAnchor somewhere in the call
   * chain.
   */

  void checkRegStateWork() const;
  void checkRegState() const { if (debug) checkRegStateWork(); }

  const ActRec* getFP()    const { checkRegState(); return m_fp; }
        ActRec* getFP()          { checkRegState(); return m_fp; }
        PC      getPC()    const { checkRegState(); return m_pc; }
  const Stack&  getStack() const { checkRegState(); return m_stack; }
        Stack&  getStack()       { checkRegState(); return m_stack; }

  Offset pcOff() const {
    return getFP()->m_func->unit()->offsetOf(m_pc);
  }

  ActRec* m_firstAR;
  std::vector<Fault> m_faults;

  ActRec* getStackFrame();
  ObjectData* getThis();
  Class* getContextClass();
  Class* getParentContextClass();
  CStrRef getContainingFileName();
  int getLine();
  Array getCallerInfo();
  bool renameFunction(const StringData* oldName, const StringData* newName);
  bool isFunctionRenameable(const StringData* name);
  void addRenameableFunctions(ArrayData* arr);
  HPHP::Eval::PhpFile* lookupPhpFile(
      StringData* path, const char* currentDir, bool* initial = nullptr);
  HPHP::Unit* evalInclude(StringData* path,
                              const StringData* curUnitFilePath, bool* initial);
  HPHP::Unit* evalIncludeRoot(StringData* path,
                                  InclOpFlags flags, bool* initial);
  HPHP::Eval::PhpFile* lookupIncludeRoot(StringData* path,
                                         InclOpFlags flags, bool* initial,
                                         HPHP::Unit* unit = 0);
  bool evalUnit(HPHP::Unit* unit, PC& pc, int funcType);
  void invokeUnit(TypedValue* retval, HPHP::Unit* unit);
  HPHP::Unit* compileEvalString(StringData* code);
  CStrRef createFunction(CStrRef args, CStrRef code);
  void evalPHPDebugger(TypedValue* retval, StringData *code, int frame);
  void enterDebuggerDummyEnv();
  void exitDebuggerDummyEnv();
  void preventReturnsToTC();
  void destructObjects();
  int m_lambdaCounter;
  struct ReentryRecord {
    VMState m_savedState;
    const ActRec* m_entryFP;
    ReentryRecord(const VMState &s, const ActRec* entryFP) :
        m_savedState(s), m_entryFP(entryFP) { }
    ReentryRecord() {}
  };
  typedef TinyVector<ReentryRecord, 32> NestedVMVec;
  NestedVMVec m_nestedVMs;

  int m_nesting;
  bool isNested() { return m_nesting != 0; }
  void pushVMState(VMState &savedVM, const ActRec* reentryAR);
  void popVMState();

  ActRec* getPrevVMState(const ActRec* fp,
                         Offset* prevPc = nullptr,
                         TypedValue** prevSp = nullptr,
                         bool* fromVMEntry = nullptr);
  Array debugBacktrace(bool skip = false,
                       bool withSelf = false,
                       bool withThis = false,
                       VMParserFrame* parserFrame = nullptr,
                       bool ignoreArgs = false,
                       int limit = 0);
  VarEnv* getVarEnv();
  void setVar(StringData* name, TypedValue* v, bool ref);
  Array getLocalDefinedVariables(int frame);
  HPHP::PCFilter* m_breakPointFilter;
  HPHP::PCFilter* m_lastLocFilter;
  bool m_dbgNoBreak;
  bool doFCall(HPHP::ActRec* ar, PC& pc);
  bool doFCallArray(PC& pc);
  CVarRef getEvaledArg(const StringData* val, CStrRef namespacedName);
private:
  void enterVMWork(ActRec* enterFnAr);
  void enterVMPrologue(ActRec* enterFnAr);
  void enterVM(TypedValue* retval, ActRec* ar);
  void reenterVM(TypedValue* retval, ActRec* ar, TypedValue* savedSP);
  void doFPushCuf(PC& pc, bool forward, bool safe);
  template <bool forwarding>
  void pushClsMethodImpl(Class* cls, StringData* name,
                         ObjectData* obj, int numArgs);
  bool prepareFuncEntry(ActRec* ar, PC& pc);
  bool prepareArrayArgs(ActRec* ar, Array& arrayArgs);
  void recordCodeCoverage(PC pc);
  bool isReturnHelper(uintptr_t address);
  void switchModeForDebugger();
  int m_coverPrevLine;
  HPHP::Unit* m_coverPrevUnit;
  Array m_evaledArgs;
public:
  void resetCoverageCounters();
  void shuffleMagicArgs(ActRec* ar);
  void syncGdbState();
  enum InvokeFlags {
    InvokeNormal = 0,
    InvokeIgnoreByRefErrors = 1,
    InvokePseudoMain = 2
  };
  void invokeFunc(TypedValue* retval,
                  const HPHP::Func* f,
                  CArrRef params,
                  ObjectData* this_ = nullptr,
                  HPHP::Class* class_ = nullptr,
                  VarEnv* varEnv = nullptr,
                  StringData* invName = nullptr,
                  InvokeFlags flags = InvokeNormal);
  void invokeFunc(TypedValue* retval,
                  const CallCtx& ctx,
                  CArrRef params,
                  VarEnv* varEnv = nullptr) {
    invokeFunc(retval, ctx.func, params, ctx.this_, ctx.cls, varEnv,
               ctx.invName, InvokeIgnoreByRefErrors);
  }
  void invokeFuncFew(TypedValue* retval,
                     const HPHP::Func* f,
                     void* thisOrCls,
                     StringData* invName,
                     int argc, TypedValue* argv);
  void invokeFuncFew(TypedValue* retval,
                     const HPHP::Func* f,
                     void* thisOrCls,
                     StringData* invName = nullptr) {
    invokeFuncFew(retval, f, thisOrCls, invName, 0, nullptr);
  }
  void invokeFuncFew(TypedValue* retval,
                     const CallCtx& ctx,
                     int argc, TypedValue* argv) {
    invokeFuncFew(retval, ctx.func,
                  ctx.this_ ? (void*)ctx.this_ :
                  ctx.cls ? (char*)ctx.cls + 1 : nullptr,
                  ctx.invName, argc, argv);
  }
  void invokeContFunc(const HPHP::Func* f,
                      ObjectData* this_,
                      Cell* param = nullptr);
  // VM ClassInfo support
  StringIMap<AtomicSmartPtr<MethodInfoVM> > m_functionInfos;
  StringIMap<AtomicSmartPtr<ClassInfoVM> >  m_classInfos;
  StringIMap<AtomicSmartPtr<ClassInfoVM> >  m_interfaceInfos;
  StringIMap<AtomicSmartPtr<ClassInfoVM> >  m_traitInfos;
  Array getUserFunctionsInfo();
  Array getConstantsInfo();
  const ClassInfo::MethodInfo* findFunctionInfo(CStrRef name);
  const ClassInfo* findClassInfo(CStrRef name);
  const ClassInfo* findInterfaceInfo(CStrRef name);
  const ClassInfo* findTraitInfo(CStrRef name);
  const ClassInfo::ConstantInfo* findConstantInfo(CStrRef name);

  // The op*() methods implement individual opcode handlers.
#define O(name, imm, pusph, pop, flags)                                       \
  void op##name();
OPCODES
#undef O
  enum DispatchFlags {
    LimitInstrs = 1 << 0,
    BreakOnCtlFlow = 1 << 1,
    Profile = 1 << 2
  };
  template <int dispatchFlags>
  void dispatchImpl(int numInstrs);
  void dispatch();
  // dispatchN() runs numInstrs instructions, or to program termination,
  // whichever comes first. If the program terminates during execution, m_pc is
  // set to null.
  void dispatchN(int numInstrs);

  // dispatchBB() tries to run until a control-flow instruction has been run.
  void dispatchBB();

private:
  static Mutex s_threadIdxLock;
  static hphp_hash_map<pid_t, int64_t> s_threadIdxMap;

public:
  static int64_t s_threadIdxCounter;
};

class ExecutionContext : public VMExecutionContext {};

#if DEBUG
#define g_vmContext (&dynamic_cast<VMExecutionContext&>( \
                       *g_context.getNoCheck()))
#else
#define g_vmContext (static_cast<VMExecutionContext*>( \
                                   static_cast<BaseExecutionContext*>( \
                                     g_context.getNoCheck())))
#endif

///////////////////////////////////////////////////////////////////////////////

class PersistentObjectStore {
public:
  ~PersistentObjectStore();

  int size() const;

  void set(const char *type, const char *name, ResourceData *obj);
  ResourceData *get(const char *type, const char *name);
  void remove(const char *type, const char *name);

  const ResourceMap &getMap(const char *type);

private:
  ResourceMapMap m_objects;

  void removeObject(ResourceData *data);
};

///////////////////////////////////////////////////////////////////////////////

extern DECLARE_THREAD_LOCAL_NO_CHECK(ExecutionContext, g_context);
extern DECLARE_THREAD_LOCAL_NO_CHECK(PersistentObjectStore,
                                     g_persistentObjects);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXECUTION_CONTEXT_H_
