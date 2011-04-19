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

#include <runtime/base/complex_types.h>
#include <runtime/base/server/transport.h>
#include <runtime/base/resource_data.h>
#include <runtime/base/fiber_safe.h>
#include <runtime/base/debuggable.h>
#include <runtime/base/server/virtual_host.h>
#include <runtime/base/util/string_buffer.h>
#include <util/thread_local.h>

#define PHP_OUTPUT_HANDLER_START  (1<<0)
#define PHP_OUTPUT_HANDLER_CONT   (1<<1)
#define PHP_OUTPUT_HANDLER_END    (1<<2)

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

///////////////////////////////////////////////////////////////////////////////
/**
 * Put all global variables here so we can gather them into one thread-local
 * variable for easy access.
 */

class ExecutionContext : public FiberLocal, public IDebuggable {
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

private:
  class OutputBuffer {
  public:
    OutputBuffer() : oss(8192) {}
    StringBuffer oss;
    Variant handler;
  };

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
  Silencer();
  ~Silencer();
  void enable();
  void disable();
  Variant disable(CVarRef v);

private:
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
