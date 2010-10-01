/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/base/fiber_async_func.h>
#include <runtime/base/program_functions.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/resource_data.h>
#include <runtime/base/fiber_reference_map.h>
#include <runtime/ext/ext_array.h>
#include <system/gen/php/globals/constants.h>
#include <util/job_queue.h>
#include <util/lock.h>
#include <util/logger.h>
#include <runtime/eval/runtime/eval_state.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
/**
 * This class provides synchronization between request thread and fiber thread
 * so to make sure when fiber job finishes after request is finished, which
 * means end_user_func_async() is forgotten, fiber job will not touch request
 * thread's data. There is no need to restore any states in this case.
 */
class FiberAsyncFuncData {
public:
  FiberAsyncFuncData() : m_reqId(0) {}
  Mutex m_mutex;
  int64 m_reqId;
};
static IMPLEMENT_THREAD_LOCAL(FiberAsyncFuncData, s_fiber_data);

void FiberAsyncFunc::OnRequestExit() {
  Lock lock(s_fiber_data->m_mutex);
  ++s_fiber_data->m_reqId;
};

///////////////////////////////////////////////////////////////////////////////

class FiberJob : public Synchronizable {
public:
  FiberJob(FiberAsyncFuncData *thread, CVarRef function, CArrRef params,
           bool async)
      : m_thread(thread), m_context(NULL),
        m_unmarshaled_function(NULL), m_unmarshaled_params(NULL),
        m_unmarshaled_global_variables(NULL),
        m_function(function), m_params(params),
        m_global_variables(NULL), m_refCount(0),
        m_async(async), m_ready(false), m_done(false), m_delete(false),
        m_exit(false), m_unmarshaled_evalState(NULL), m_evalState(NULL) {
    m_reqId = m_thread->m_reqId;

    // Profoundly needed: (1) to make sure references and objects are held
    // when job finishes, as otherwise, caller can release its last reference
    // to destruct them, then unmarshal coding will fail. (2) to make sure
    // references have refcount > 1, which is needed by
    // Variant::fiberUnmarshal() code to tell who needs to set back to original
    // reference.
    if (m_async) {
      m_context = g_context.get();
      ASSERT(m_context);
      m_unmarshaled_function = NEW(Variant)();
      *m_unmarshaled_function = m_function;
      m_unmarshaled_params = NEW(Variant)();
      *m_unmarshaled_params = m_params;
      m_unmarshaled_global_variables = get_global_variables();
      m_unmarshaled_evalState = Eval::RequestEvalState::Get();
    }
  }

  ~FiberJob() {
  }

  void cleanup() {
    if (m_unmarshaled_function) {
      Lock lock(m_thread->m_mutex);
      if (m_thread->m_reqId == m_reqId) {
        DELETE(Variant)(m_unmarshaled_function);
        DELETE(Variant)(m_unmarshaled_params);
        m_unmarshaled_function = NULL;
        m_unmarshaled_params = NULL;
      }
      // else not safe to touch these members because thread has moved to
      // next request after deleting/collecting all these dangling ones
    }
  }

  void waitForReady() {
    Lock lock(this);
    while (!m_ready) wait();
  }

  bool isDone() {
    return m_done;
  }

  bool canDelete() {
    return m_delete && m_refCount == 1;
  }

  void run() {
    // make local copy of m_function and m_params
    if (m_async) {
      ExecutionContext *context = g_context.get();
      if (context && m_context) {
        context->fiberInit(m_context, m_refMap);
        m_context = context; // switching role
      }
      (m_evalState = Eval::RequestEvalState::Get())->
        fiberInit(m_unmarshaled_evalState, m_refMap);
      m_function = m_function.fiberMarshal(m_refMap);
      m_params = m_params.fiberMarshal(m_refMap);
      ThreadInfo::s_threadInfo->m_globals =
        m_global_variables = get_global_variables();
      fiber_marshal_global_state(m_global_variables,
                                 m_unmarshaled_global_variables, m_refMap);

      Lock lock(this);
      m_ready = true;
      notify();
    }

    try {
      m_return = f_call_user_func_array(m_function, m_params);
    } catch (const ExitException &e) {
      m_exit = true;
    } catch (const Exception &e) {
      m_fatal = String(e.getMessage());
    } catch (Object e) {
      m_exception = e;
    } catch (...) {
      m_fatal = String("unknown exception was thrown");
    }

    Lock lock(this);
    m_done = true;
    notify();
  }

  Variant syncGetResults() {
    if (m_exit) {
      throw ExitException(0);
    }
    if (!m_fatal.isNull()) {
      throw FatalErrorException(m_fatal.data());
    }
    if (!m_exception.isNull()) {
      throw m_exception;
    }
    return m_return;
  }

  Variant getResults(FiberAsyncFunc::Strategy default_strategy,
                     vector<pair<string, char> > &resolver) {
    if (!m_async) return syncGetResults();

    {
      Lock lock(this);
      while (!m_done) wait();
    }

    ExecutionContext *context = g_context.get();
    if (context && m_context) {
      context->fiberExit(m_context, m_refMap);
      m_context = NULL;
    }

    Eval::RequestEvalState::Get()->fiberExit(m_evalState, m_refMap,
                                             default_strategy);
    fiber_unmarshal_global_state(get_global_variables(), m_global_variables,
                                 m_refMap, default_strategy, resolver);

    // these are needed in case they have references or objects
    if (!m_refMap.empty()) {
      m_function.fiberUnmarshal(m_refMap);
      m_params.fiberUnmarshal(m_refMap);
    }

    Object unmarshaled_exception = m_exception.fiberUnmarshal(m_refMap);
    Variant unmarshaled_return = m_return.fiberUnmarshal(m_refMap);
    try {
      if (m_exit) {
        throw ExitException(0);
      }
      if (!m_fatal.isNull()) {
        throw FatalErrorException(m_fatal.data());
      }
      if (!m_exception.isNull()) {
        throw unmarshaled_exception;
      }
    } catch (...) {
      cleanup();
      m_delete = true;
      throw;
    }

    cleanup();
    m_delete = true;
    return unmarshaled_return;
  }

  // ref counting
  void incRefCount() {
    atomic_inc(m_refCount);
  }
  void decRefCount() {
    ASSERT(m_refCount);
    if (atomic_dec(m_refCount) == 0) {
      delete this;
    }
  }

private:
  FiberAsyncFuncData *m_thread;

  // holding references to them, so we can later restore their states safely
  ExecutionContext *m_context; // peer thread's
  Variant *m_unmarshaled_function;
  Variant *m_unmarshaled_params;
  GlobalVariables *m_unmarshaled_global_variables;

  FiberReferenceMap m_refMap;
  int64 m_reqId;

  Variant m_function;
  Array m_params;
  GlobalVariables *m_global_variables;

  int m_refCount;

  bool m_async;
  bool m_ready;
  bool m_done;
  bool m_delete;

  bool m_exit;
  String m_fatal;
  Object m_exception;
  Variant m_return;
  Eval::RequestEvalState *m_unmarshaled_evalState;
  Eval::RequestEvalState *m_evalState;
};

///////////////////////////////////////////////////////////////////////////////

class FiberWorker : public JobQueueWorker<FiberJob*> {
public:
  FiberWorker() {
    hphp_session_init();
    hphp_context_init();
  }

  ~FiberWorker() {
    hphp_context_exit(g_context.get(), false, false);
    hphp_session_exit();
  }

  virtual void doJob(FiberJob *job) {
    job->run();
    m_jobs.push_back(job);
    cleanup();
  }

  void cleanup() {
    list<FiberJob*>::iterator iter = m_jobs.begin();
    while (iter != m_jobs.end()) {
      FiberJob *job = *iter;
      if (job->canDelete()) {
        job->decRefCount();
        iter = m_jobs.erase(iter);
        continue;
      }
      ++iter;
    }

    // Finally we can take a breath releasing some memory. It's still possible
    // we're suffocated to death, but in reality, fiber threads should be able
    // to take breaks from time to time, or the count should be increased.
    if (m_jobs.empty()) {
      hphp_context_exit(g_context.get(), false, true);
      hphp_session_exit();

      hphp_session_init();
      hphp_context_init();
    }
  }

private:
  list<FiberJob*> m_jobs;
};

///////////////////////////////////////////////////////////////////////////////

class FiberAsyncFuncHandle : public SweepableResourceData {
public:
  DECLARE_OBJECT_ALLOCATION(FiberAsyncFuncHandle)

  FiberAsyncFuncHandle(CVarRef function, CArrRef params, bool async) {
    m_job = new FiberJob(s_fiber_data.get(), function, params, async);
    m_job->incRefCount();
  }

  ~FiberAsyncFuncHandle() {
    m_job->decRefCount();
  }

  FiberJob *getJob() { return m_job;}

  static StaticString s_class_name;
  // overriding ResourceData
  virtual CStrRef o_getClassName() const { return s_class_name; }

private:
  FiberJob *m_job;
};

IMPLEMENT_OBJECT_ALLOCATION(FiberAsyncFuncHandle)

StaticString FiberAsyncFuncHandle::s_class_name("FiberAsyncFuncHandle");

///////////////////////////////////////////////////////////////////////////////

static JobQueueDispatcher<FiberJob*, FiberWorker> *s_dispatcher;

void FiberAsyncFunc::Restart() {
  if (s_dispatcher) {
    s_dispatcher->stop();
    delete s_dispatcher;
    s_dispatcher = NULL;
  }
  if (RuntimeOption::FiberCount > 0) {
    s_dispatcher = new JobQueueDispatcher<FiberJob*, FiberWorker>
      (RuntimeOption::FiberCount, RuntimeOption::ServerThreadRoundRobin,
       RuntimeOption::ServerThreadDropCacheTimeoutSeconds, NULL);
    Logger::Verbose("fiber job dispatcher started");
    s_dispatcher->start();
  }
}

Object FiberAsyncFunc::Start(CVarRef function, CArrRef params) {
  FiberAsyncFuncHandle *handle =
    NEW(FiberAsyncFuncHandle)(function, params, s_dispatcher != NULL);
  Object ret(handle);

  FiberJob *job = handle->getJob();
  if (s_dispatcher) {
    job->incRefCount(); // paired with worker's decRefCount()
    s_dispatcher->enqueue(job);
    job->waitForReady(); // until job data are copied into fiber
  } else {
    job->run(); // immediately executing the job
  }

  return ret;
}

bool FiberAsyncFunc::Status(CObjRef func) {
  FiberAsyncFuncHandle *handle = func.getTyped<FiberAsyncFuncHandle>();
  return handle->getJob()->isDone();
}

Variant FiberAsyncFunc::Result(CObjRef func, Strategy default_strategy,
                               CVarRef additional_strategies) {
  FiberAsyncFuncHandle *handle = func.getTyped<FiberAsyncFuncHandle>();
  if (!additional_strategies.isArray() && !additional_strategies.isNull()) {
    raise_error("additional strategies need to be an array");
  }

  vector<pair<string, char> > resolver;
  resolver.reserve(f_count(additional_strategies, true));

  for (ArrayIter iter(additional_strategies); iter; ++iter) {
    string prefix;
    int kind = iter.first().toInt32();
    if (kind == k_GLOBAL_SYMBOL_GLOBAL_VARIABLE) {
      prefix = "gv_"; // Option::GlobalVariablePrefix;
    } else if (kind == k_GLOBAL_SYMBOL_STATIC_VARIABLE) {
      prefix = "sv_"; // Option::StaticVariablePrefix;
    } else if (kind == k_GLOBAL_SYMBOL_CLASS_STATIC) {
      prefix = "s_";  // Option::StaticPropertyPrefix;
    } else {
      raise_error("Unknown global symbol type: %d", kind);
    }

    Variant symbols = iter.second();
    if (!symbols.isArray()) {
      raise_error("additional strategies need to be an array of arrays");
    }
    for (ArrayIter iter2(symbols); iter2; ++iter2) {
      string name = prefix + iter2.first().toString().data();
      resolver.push_back(pair<string, char>(name, iter2.second().toByte()));
    }
  }

  return handle->getJob()->getResults(default_strategy, resolver);
}

///////////////////////////////////////////////////////////////////////////////
}
