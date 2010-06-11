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
#include <runtime/base/builtin_functions.h>
#include <runtime/base/resource_data.h>
#include <util/job_queue.h>
#include <util/lock.h>
#include <util/logger.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class FiberJob : public Synchronizable {
public:
  FiberJob(CVarRef function, CArrRef params)
      : m_function(function), m_params(params), m_refCount(0),
        m_async(true), m_ready(false), m_done(false), m_exit(false) {
  }

  void waitForReady() {
    Lock lock(this);
    while (!m_ready) wait();
  }

  bool isDone() {
    return m_done;
  }

  void run(bool async) {
    m_async = async;

    // make local copy of m_function and m_params
    if (async) {
      m_function = m_function.fiberCopy();
      m_params = m_params.fiberCopy();

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

  Variant getResults(FiberAsyncFunc::Strategy strategy, CVarRef resolver) {
    {
      Lock lock(this);
      while (!m_done) wait();
    }

    if (m_exit) {
      throw ExitException(0);
    }
    if (!m_fatal.isNull()) {
      throw FatalErrorException("%s", m_fatal.data());
    }
    if (!m_exception.isNull()) {
      if (m_async) {
        throw m_exception.fiberCopy();
      } else {
        throw m_exception;
      }
    }
    if (m_async) {
      return m_return.fiberCopy();
    }
    return m_return;
  }

  // ref counting
  void incRefCount() {
    Lock lock(m_mutex);
    ++m_refCount;
  }
  void decRefCount() {
    {
      Lock lock(m_mutex);
      --m_refCount;
    }
    if (m_refCount == 0) {
      delete this;
    }
  }

private:
  Variant m_function;
  Array m_params;

  Mutex m_mutex;
  int m_refCount;

  bool m_async;
  bool m_ready;
  bool m_done;

  bool m_exit;
  String m_fatal;
  Object m_exception;
  Variant m_return;
};

///////////////////////////////////////////////////////////////////////////////

class FiberWorker : public JobQueueWorker<FiberJob*> {
public:
  virtual void doJob(FiberJob *job) {
    job->run(true);
    job->decRefCount();
  }
};

///////////////////////////////////////////////////////////////////////////////

class FiberAsyncFuncHandle : public ResourceData {
public:
  DECLARE_OBJECT_ALLOCATION(FiberAsyncFuncHandle);

  FiberAsyncFuncHandle(CVarRef function, CArrRef params) {
    m_job = new FiberJob(function, params);
    m_job->incRefCount();
  }

  ~FiberAsyncFuncHandle() {
    m_job->decRefCount();
  }

  FiberJob *getJob() { return m_job;}

  // overriding ResourceData
  virtual const char *o_getClassName() const { return "FiberAsyncFuncHandle";}

private:
  FiberJob *m_job;
};

IMPLEMENT_OBJECT_ALLOCATION(FiberAsyncFuncHandle);

///////////////////////////////////////////////////////////////////////////////
// implementing PageletServer

static JobQueueDispatcher<FiberJob*, FiberWorker> *s_dispatcher;

void FiberAsyncFunc::Restart() {
  if (s_dispatcher) {
    s_dispatcher->stop();
    delete s_dispatcher;
    s_dispatcher = NULL;
  }
  if (RuntimeOption::FiberCount > 0) {
    s_dispatcher = new JobQueueDispatcher<FiberJob*, FiberWorker>
      (RuntimeOption::FiberCount, NULL);
    Logger::Info("fiber job dispatcher started");
    s_dispatcher->start();
  }
}

Object FiberAsyncFunc::Start(CVarRef function, CArrRef params) {
  FiberAsyncFuncHandle *handle = NEW(FiberAsyncFuncHandle)(function, params);
  Object ret(handle);

  FiberJob *job = handle->getJob();
  if (s_dispatcher) {
    job->incRefCount(); // paired with worker's decRefCount()
    s_dispatcher->enqueue(job);
    job->waitForReady(); // until job data are copied into fiber
  } else {
    job->run(false); // immediately executing the job
  }

  return ret;
}

bool FiberAsyncFunc::Status(CObjRef func) {
  FiberAsyncFuncHandle *handle = func.getTyped<FiberAsyncFuncHandle>();
  return handle->getJob()->isDone();
}

Variant FiberAsyncFunc::Result(CObjRef func, Strategy strategy,
                               CVarRef resolver) {
  FiberAsyncFuncHandle *handle = func.getTyped<FiberAsyncFuncHandle>();
  return handle->getJob()->getResults(strategy, resolver);
}

///////////////////////////////////////////////////////////////////////////////

static IMPLEMENT_THREAD_LOCAL(PointerMap, s_forward_references);
static IMPLEMENT_THREAD_LOCAL(PointerMap, s_reverse_references);

void *FiberReferenceMap::Lookup(void *src) {
  PointerMap::iterator iter = s_forward_references->find(src);
  if (iter != s_forward_references->end()) {
    return iter->second;
  }
  return NULL;
}

void *FiberReferenceMap::ReverseLookup(void *copy) {
  PointerMap::iterator iter = s_reverse_references->find(copy);
  if (iter != s_reverse_references->end()) {
    return iter->second;
  }
  return NULL;
}

void FiberReferenceMap::Insert(void *src, void *copy) {
  ASSERT(Lookup(src) == NULL);
  ASSERT(copy == NULL || ReverseLookup(copy) == NULL);
  (*s_forward_references)[src] = copy;
  if (copy) {
    (*s_reverse_references)[copy] = src;
  }
}

///////////////////////////////////////////////////////////////////////////////
}
