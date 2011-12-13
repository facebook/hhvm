/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __HPHP_EXECUTION_CONTEXT_H__
#define __HPHP_EXECUTION_CONTEXT_H__

#include <runtime/base/class_info.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/server/transport.h>
#include <runtime/base/resource_data.h>
#include <runtime/base/fiber_safe.h>
#include <runtime/base/debuggable.h>
#include <runtime/base/server/virtual_host.h>
#include <runtime/base/util/string_buffer.h>
#include <runtime/base/array/hphp_array.h>
#include <runtime/vm/funcdict.h>
#include <runtime/vm/func.h>
#include <runtime/vm/bytecode.h>
#include <runtime/vm/translator/translator.h>
#include <runtime/vm/instrumentation.h>
#include <util/base.h>
#include <util/lock.h>
#include <util/thread_local.h>
#include <setjmp.h>

#include <runtime/eval/base/ast_ptr.h>

#define PHP_OUTPUT_HANDLER_START  (1<<0)
#define PHP_OUTPUT_HANDLER_CONT   (1<<1)
#define PHP_OUTPUT_HANDLER_END    (1<<2)

namespace HPHP {
namespace Eval {
class PhpFile;
}
namespace VM {
class DynTracer;
}
}

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

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
  HPHP::VM::PC pc;
  HPHP::VM::ActRec* fp;
  HPHP::VM::ActRec* firstAR;
  TypedValue *sp;
};

class MethodInfoVM : public ClassInfo::MethodInfo,
                     public HPHP::Eval::AtomicCountable {
 public:
  ~MethodInfoVM();
  void release() { delete this; }
};

class ClassInfoVM : public ClassInfo,
                    public HPHP::Eval::AtomicCountable {
 public:
  ~ClassInfoVM();
  void release() { delete this; }
  virtual CStrRef getParentClass() const { return m_parentClass; }

  const InterfaceSet  &getInterfaces()      const { return m_interfaces;}
  const InterfaceVec  &getInterfacesVec()   const { return m_interfacesVec;}
  const MethodMap     &getMethods()         const { return m_methods;}
  const MethodVec     &getMethodsVec()      const { return m_methodsVec;}
  const PropertyMap   &getProperties()      const { return m_properties;}
  const PropertyVec   &getPropertiesVec()   const { return m_propertiesVec;}
  const ConstantMap   &getConstants()       const { return m_constants;}
  const ConstantVec   &getConstantsVec()    const { return m_constantsVec;}
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

 public:
  friend class HPHP::VM::Class;
};

namespace MethodLookup {
enum CallType {
  ClsMethod,
  ObjMethod,
};
enum LookupResult {
  MethodFoundWithThis,
  MethodFoundNoThis,
  MagicCallFound,
  MagicCallStaticFound,
  MethodNotFound,
};
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

class ExecutionContext : public FiberLocal, public IDebuggable {
public:
#define NEAR_FIELD_DECL                                                        \
  HPHP::VM::Stack m_stack;                                                     \
  HPHP::VM::ActRec* m_fp;                                                      \
  HPHP::VM::PC m_pc;                                                           \
  uint32 m_isValid; /* Debug-only: non-zero iff m_fp/m_stack are trustworthy */\
  HPHP::VM::DynTracer* m_dynTracer;                                            \
  int64_t m_currentThreadIdx;
#ifdef HHVM
  // These members are declared first for performance reasons: they
  // are accessed from within the TC and having their offset fit
  // within a single byte makes the generated code slightly smaller
  // and faster.
  NEAR_FIELD_DECL
#endif

public:
  enum ShutdownType {
    ShutDown,
    PostSend,
    CleanUp,

    ShutdownTypeCount
  };

  enum ErrorThrowMode {
    NeverThrow,
    ThrowIfUnhandled,
    AlwaysThrow,
  };

  enum ErrorState {
    NoError,
    ErrorRaised,
    ExecutingUserHandler,
    ErrorRaisedByUserHandler,
  };

public:
  ExecutionContext();
  ~ExecutionContext();

  // For RPCRequestHandler
  void backupSession();
  void restoreSession();

  // implementing FiberLocal
  virtual void fiberInit(FiberLocal *src, FiberReferenceMap &refMap);
  virtual void fiberExit(FiberLocal *src, FiberReferenceMap &refMap);

  // implementing IDebuggable
  virtual void debuggerInfo(InfoVec &info);

  /**
   * System settings.
   */
  Transport *getTransport() { return m_transport;}
  void setTransport(Transport *transport) { m_transport = transport;}
  String getMimeType() const;
  void setContentType(CStrRef mimetype, CStrRef charset);
  int64 getRequestMemoryMaxBytes() const { return m_maxMemory; }
  void setRequestMemoryMaxBytes(int64 max);
  int64 getRequestTimeLimit() const { return m_maxTime; }
  void setRequestTimeLimit(int64 limit) { m_maxTime = limit;}
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
  void obStart(CVarRef handler = null);
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
  void registerTickFunction(CVarRef function, Array arguments);
  void unregisterTickFunction(CVarRef function);
  void onTick();
  Array backupShutdowns() const { return m_shutdowns;}
  void restoreShutdowns(CArrRef shutdowns) { m_shutdowns = shutdowns;}

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
                   const std::string &prefix);
  bool callUserErrorHandler(const Exception &e, int errnum,
                            bool swallowExceptions);
  void recordLastError(const Exception &e, int errnum = 0);
  bool onFatalError(const Exception &e); // returns handled
  bool onUnhandledException(Object e);
  int getErrorState() const { return m_errorState;}
  void setErrorState(int state) { m_errorState = state;}
  String getLastError() const { return m_lastError;}
  int getLastErrorNumber() const { return m_lastErrorNum;}
  int getErrorReportingLevel() const { return m_errorReportingLevel;}
  void setErrorReportingLevel(int level) { m_errorReportingLevel = level;}
  String getErrorPage() const { return m_errorPage;}
  void setErrorPage(CStrRef page) { m_errorPage = page;}
  bool getLogErrors() const { return m_logErrors;}
  void setLogErrors(bool on);
  String getErrorLog() const { return m_errorLog;}
  void setErrorLog(CStrRef filename);

  /**
   * Misc. settings
   */
  String getenv(CStrRef name) const;
  void setenv(CStrRef name, CStrRef value);
  String getTimeZone() const { return m_timezone;}
  void setTimeZone(CStrRef timezone) { m_timezone = timezone;}
  String getDefaultTimeZone() const { return m_timezoneDefault;}
  String getArgSeparatorOutput() const {
    if (m_argSeparatorOutput.isNull()) return "&";
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

  DECLARE_DBG_SETTING_ACCESSORS

  VM::Transl::Translator* __getTransl() {
    // Only for debugging purposes; please do not abuse.
    return m_transl;
  }

private:
  class OutputBuffer {
  public:
    OutputBuffer() : oss(8192) {}
    StringBuffer oss;
    Variant handler;
  };

public:
  typedef hphp_hash_set<HPHP::ObjectData*, pointer_hash<HPHP::ObjectData> >
    LiveObjSet;
  LiveObjSet m_liveBCObjs;
private:
  VM::Transl::Translator* m_transl;
  // system settings
  Transport *m_transport;
  int64 m_maxMemory;
  int64 m_maxTime;
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
  Array m_ticks;

  // error handling
  std::vector<std::pair<Variant,int> > m_userErrorHandlers;
  std::vector<Variant> m_userExceptionHandlers;
  int m_errorState;
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

  const VirtualHost *m_vhost;
  // helper functions
  void resetCurrentBuffer();
  void executeFunctions(CArrRef funcs);

  DECLARE_DBG_SETTING

  // HHBC-related state/methods
public:
  void requestInit();
  void requestExit();

private:
  template <bool warn, bool saveResult, unsigned mleave>
  void getHelperPre(HPHP::VM::PC& pc, unsigned& ndiscard, TypedValue*& tvL,
                    TypedValue*& base, bool& baseStrOff, TypedValue& tvScratch,
                    TypedValue& tvRef, TypedValue& tvRef2);
  template <bool saveResult>
  void getHelperPost(unsigned ndiscard, TypedValue* tvL, TypedValue& tvScratch,
                     TypedValue& tvRef, TypedValue& tvRef2);
  void getHelper(HPHP::VM::PC& pc, unsigned& ndiscard, TypedValue*& tvL,
                 TypedValue*& base, bool& baseStrOff, TypedValue& tvScratch,
                 TypedValue& tvRef, TypedValue& tvRef2);

  template <bool warn, bool define, bool unset, unsigned mdepth,
            unsigned mleave>
  bool setHelperPre(HPHP::VM::PC& pc, unsigned& ndiscard, TypedValue*& tvL,
                    HPHP::VM::PC& vecp, TypedValue*& base, bool& baseStrOff,
                    TypedValue& tvScratch, TypedValue& tvRef,
                    TypedValue& tvRef2);
  template <unsigned mdepth>
  void setHelperPost(unsigned ndiscard, TypedValue* tvL, TypedValue& tvRef,
                     TypedValue& tvRef2);
  bool cellInstanceOfStr(TypedValue* c, StringData* s);
#define O(name, imm, pusph, pop, flags)                                       \
  void iop##name(HPHP::VM::PC& pc);
OPCODES
#undef O
  void pushLocalsAndIterators(const HPHP::VM::Func* f, int nparams = 0);

public:
  HphpArray m_constants;
  typedef hphp_hash_map<const StringData*, ClassInfo::ConstantInfo*,
                        string_data_hash, string_data_same> ConstInfoMap;
  ConstInfoMap m_constInfo;
  typedef hphp_hash_map<const StringData*, HPHP::VM::Class*, string_data_hash,
                        string_data_isame> DefinedClassMap;
  DefinedClassMap m_definedClasses;
  typedef hphp_hash_map<const HPHP::VM::Class*, HphpArray*,
                        pointer_hash<HPHP::VM::Class> > ClsCnsDataMap;
  ClsCnsDataMap m_clsCnsData;
  typedef hphp_hash_map<const HPHP::VM::Class*, HPHP::VM::Class::PropInitVec*,
                        pointer_hash<HPHP::VM::Class> > PropDataMap;
  PropDataMap m_propData;
  typedef hphp_hash_map<const HPHP::VM::Class*, HphpArray*,
                        pointer_hash<HPHP::VM::Class> > SPropDataMap;
  SPropDataMap m_sPropData;
  typedef hphp_hash_map<const HPHP::VM::Class*, HphpArray*,
                        pointer_hash<HPHP::VM::Class> > ClsStaticCtxMap;
  ClsStaticCtxMap m_clsStaticCtx;

  HPHP::VM::Class* defClass(HPHP::VM::PreClass* preClass,
                            bool failIsFatal = true);

  // lookupClass --
  //   Get this request's understanding of the named class,
  //   without perturbing the state of the class mapping.
  //
  // loadClass --
  //   Like lookupClass, but may optionally invoke the autload
  //   machinery.
  //
  // getClass --
  //   Parameterized loadClass vs. getClass, since some code
  //   paths need one semantic or the other from common code.
  HPHP::VM::Class* getClass(const StringData* className,
                            bool tryAutoload);
  HPHP::VM::Class* loadClass(const StringData* className) {
    return getClass(className, true);
  }
  HPHP::VM::Class* lookupClass(const StringData* className) {
    return getClass(className, false);
  }

  const HPHP::VM::Func* lookupMethodCtx(const HPHP::VM::Class* cls,
                                        const StringData* methodName,
                                        HPHP::VM::PreClass* pctx,
                                        MethodLookup::CallType lookupType,
                                        bool raise = false);
  MethodLookup::LookupResult lookupObjMethod(const HPHP::VM::Func*& f,
                                             const HPHP::VM::Class* cls,
                                             const StringData* methodName,
                                             bool raise = false);
  MethodLookup::LookupResult lookupClsMethod(const HPHP::VM::Func*& f,
                                             const HPHP::VM::Class* cls,
                                             const StringData* methodName,
                                             ObjectData* this_,
                                             bool raise = false);
  HPHP::ObjectData* createObject(StringData* clsName,
                                 CArrRef params,
                                 bool init = true);
  HPHP::ObjectData* createObjectOnly(StringData* clsName);
  HPHP::VM::Class::PropInitVec* getPropData(const HPHP::VM::Class* class_)
    const {
    PropDataMap::const_iterator it = m_propData.find(class_);
    if (it == m_propData.end()) {
      return NULL;
    }
    return it->second;
  }
  void setPropData(const HPHP::VM::Class* class_,
                    HPHP::VM::Class::PropInitVec* propData) {
    ASSERT(getSPropData(class_) == NULL);
    m_propData[class_] = propData;
  }

  HphpArray* getSPropData(const HPHP::VM::Class* class_) const {
    SPropDataMap::const_iterator it = m_sPropData.find(class_);
    if (it == m_sPropData.end()) {
      return NULL;
    }
    return it->second;
  }
  void setSPropData(const HPHP::VM::Class* class_, HphpArray* sPropData) {
    ASSERT(getSPropData(class_) == NULL);
    m_sPropData[class_] = sPropData;
  }

  HphpArray* getClsStaticCtx(const HPHP::VM::Class* class_) {
    ClsStaticCtxMap::iterator it = m_clsStaticCtx.find(class_);
    if (it != m_clsStaticCtx.end()) {
      return it->second;
    }

    HphpArray* array = NEW(HphpArray)(1);
    array->incRefCount();
    return m_clsStaticCtx[class_] = array;
  }

  HphpArray* getClsCnsData(const HPHP::VM::Class* class_) const {
    ClsCnsDataMap::const_iterator it = m_clsCnsData.find(class_);
    if (it == m_clsCnsData.end()) {
      return NULL;
    }
    return it->second;
  }
  void setClsCnsData(const HPHP::VM::Class* class_, HphpArray* clsCnsData) {
    ASSERT(getClsCnsData(class_) == NULL);
    m_clsCnsData[class_] = clsCnsData;
  }

  HPHP::VM::ActRec* arGetSfp(const HPHP::VM::ActRec* ar);

  std::string prettyStack(const std::string& prefix) const;
  static void DumpStack();

  std::list<HPHP::VM::VarEnv*> m_varEnvs;
  // The conceptual type of this array is
  // [mangled Func* and variable name] => [value]
  // Objects that need their own static local context separate from
  // this one (Class, Closure, Continuation) are responsible for
  // managing that on their own.
  HphpArray* m_staticVars;

  HPHP::VM::FuncDict m_funcDict;
  typedef hphp_hash_map<StringData*, HPHP::Eval::PhpFile*, string_data_hash,
                        string_data_same> EvaledFilesMap;
  EvaledFilesMap m_evaledFiles;
  typedef std::vector<HPHP::VM::Unit*> EvaledUnitsVec;
  EvaledUnitsVec m_evaledUnits;

#ifndef HHVM
  NEAR_FIELD_DECL
#endif
#undef NEAR_FIELD_ECL
  HPHP::VM::ActRec* m_firstAR;
  bool m_halted;
  std::vector<HPHP::VM::Fault> m_faults;

  HPHP::VM::ActRec* getStackFrame(int level);
  ObjectData* getThis(bool skipFrame = false);
  CStrRef getContextClassName(bool skipFrame = false);
  CStrRef getParentContextClassName(bool skipFrame = false);
  CStrRef getContainingFileName(bool skipFrame = false);
  int getLine(bool skipFrame = false);
  Array getCallerInfo(bool skipFrame = false);
  bool defined(CStrRef name);
  TypedValue* getCns(StringData* cns, bool system=true, bool dynamic=true);
  bool setCns(StringData* cns, CVarRef val);
  HPHP::VM::Func *lookupFunc(const StringData* funcName);
  bool renameFunction(const StringData* oldName, const StringData* newName);
  bool isFunctionRenameable(const StringData* name);
  void addRenameableFunctions(ArrayData* arr);
  void mergeUnit(HPHP::VM::Unit* unit);
  void mergeUnitFuncs(HPHP::VM::Unit* unit);
  HPHP::Eval::PhpFile* lookupPhpFile(
      StringData* path, const char* currentDir, bool& initial);
  HPHP::VM::Unit* evalInclude(StringData* path,
                              const StringData* curUnitFilePath, bool& initial);
  void evalUnit(HPHP::VM::Unit* unit, HPHP::VM::PC& pc);
  void invokeUnit(HPHP::VM::Unit* unit);
  void evalPHPDebugger(TypedValue* retval, StringData *code, int frame);
  void enterDebuggerDummyEnv();
  void exitDebuggerDummyEnv();
  void destructObjects();
  bool getCallInfo(const CallInfo *&ci, void *&extra, const char *s);
  bool getCallInfoStatic(const CallInfo *&ci, void *&extra,
                         const StringData *cls, const StringData *func);
  int m_lambdaCounter;
  std::vector<jmp_buf *> m_jmpBufs;
  std::vector<VMState> m_nestedVMs;

  typedef hphp_hash_map<const HPHP::VM::ActRec*, int,
                        pointer_hash<HPHP::VM::ActRec> > NestedVMMap;
  NestedVMMap m_nestedVMMap; // Given an ActRec* whose previous frame is a
                             // native frame, this function will give the
                             // index into m_nestedVMs corresponding to the
                             // previous VM.

  int m_nesting;
  bool isNested() { return m_nesting != 0; }
  void reenterVM(VMState &savedVM);
  void exitVM();

  bool m_propagateException;
  bool m_ignoreException;
  void hhvmThrow();
  HPHP::VM::ActRec* getPrevVMState(const HPHP::VM::ActRec* fp,
                                   HPHP::VM::Offset* prevPc = NULL,
                                   TypedValue** prevSp = NULL);
  Array debugBacktrace(bool skip = false,
                       bool withSelf = false,
                       bool withThis = false,
                       VMParserFrame* parserFrame = NULL);
  void handleUnwind(VM::UnwindStatus unwindType);
  HPHP::VM::VarEnv* getVarEnv();
  void setVar(StringData* name, TypedValue* v, bool ref);
  Array getLocalDefinedVariables(int frame);
  bool m_debuggerFuncEntry;
  const HPHP::VM::SourceLoc *m_debuggerLastBreakLoc;
  HPHP::VM::InjectionTables* m_injTables;
private:
  void shuffleMagicArgs(HPHP::VM::ActRec* ar, int nargs);
  void callBuiltin(HPHP::VM::ActRec* ar);
  template <bool checkOverflow>
  bool prepareNonBuiltinEntry(HPHP::VM::ActRec *ar, HPHP::VM::PC& pc);
  void handleExit();
public:
  void syncGdbState();
  void enterVM(TypedValue* retval,
               const HPHP::VM::Func* f,
               HPHP::VM::VarEnv* varEnv,
               HPHP::VM::ActRec* ar);
  void invokeFunc(TypedValue* retval,
                  const HPHP::VM::Func* f,
                  CArrRef params,
                  ObjectData* this_ = NULL,
                  HPHP::VM::Class* class_ = NULL,
                  HPHP::VM::VarEnv* varEnv = NULL,
                  StringData* invName = NULL,
                  HPHP::VM::Unit* mergeUnit = NULL);
  // VM ClassInfo support
  StringIMap<SmartPtr<MethodInfoVM> > m_functionInfos;
  StringIMap<SmartPtr<ClassInfoVM> >  m_classInfos;
  StringIMap<SmartPtr<ClassInfoVM> >  m_interfaceInfos;
  StringIMap<SmartPtr<ClassInfoVM> >  m_traitInfos;
  Array getUserFunctionsInfo();
  Array getClassesInfo();
  Array getInterfacesInfo();
  Array getTraitsInfo();
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
  template <bool limInstrs, bool breakOnCtlFlow>
  void dispatchImpl(int numInstrs);
  void dispatch();
  // dispatchN() runs numInstrs instructions, or to program termination,
  // whichever comes first. If the program terminates during execution, m_pc is
  // set to null.
  void dispatchN(int numInstrs);

  // dispatchBB() tries to run until a control-flow instruction has been run.
  void dispatchBB();

  inline bool isHalted() { return m_halted; }

private:
  static Mutex s_threadIdxLock;
  static hphp_hash_map<pid_t, int64_t> s_threadIdxMap;

public:
  static int64_t s_threadIdxCounter;
  inline HPHP::VM::Offset pcOff() const {
    return m_fp->m_func->m_unit->offsetOf(m_pc);
  }
};

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

class Silencer {
public:
  Silencer() : m_active(false) {}
  Silencer(bool);

  ~Silencer() { if (m_active) disableHelper(); }
  void enable();
  void disable() { disableHelper(); m_active = false; }
  Variant disable(CVarRef v);

private:
  void disableHelper();
  bool m_active;
  int m_errorReportingValue;
};

///////////////////////////////////////////////////////////////////////////////

extern DECLARE_THREAD_LOCAL_NO_CHECK(ExecutionContext, g_context);
extern DECLARE_THREAD_LOCAL_NO_CHECK(PersistentObjectStore,
                                     g_persistentObjects);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_EXECUTION_CONTEXT_H__
