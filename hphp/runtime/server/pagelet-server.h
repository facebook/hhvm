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

#ifndef incl_HPHP_PAGELET_SERVER_H_
#define incl_HPHP_PAGELET_SERVER_H_

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/server/transport.h"
#include "hphp/runtime/server/server-task-event.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class PageletTransport;
class PageletServerTaskEvent;

class PageletServer {
public:
  static bool Enabled();
  static void Restart();
  static void Stop();

  /**
   * Create a task. This returns a task handle, or null object
   * if there are no worker threads.
   */
  static Resource TaskStart(
    const String& url, CArrRef headers,
    const String& remote_host,
    const String& post_data = null_string,
    CArrRef files = null_array,
    int timeoutSeconds = -1,
    PageletServerTaskEvent *event = nullptr
  );

  /**
   * Query if a task is finished. This is non-blocking and can be called as
   * many times as desired.
   */
  static int64_t TaskStatus(CResRef task);

  /**
   * Get results of a task. This is blocking until task is finished or times
   * out. The status code is set to -1 in the event of a timeout.
   */
  static String TaskResult(CResRef task,
                           Array &headers,
                           int &code,
                           int64_t timeout_ms);

  /**
   * Add a piece of response to the pipeline.
   */
  static void AddToPipeline(const std::string &s);

  /**
   * Check active threads and queued requests
   */
  static int GetActiveWorker();
  static int GetQueuedJobs();
};

class PageletTransport : public Transport, public Synchronizable {
public:
  PageletTransport(
    const String& url, CArrRef headers, const String& postData,
    const String& remoteHost, const std::set<string> &rfc1867UploadedFiles,
    CArrRef files, int timeoutSeconds);

  /**
   * Implementing Transport...
   */
  virtual const char *getUrl();
  virtual const char *getRemoteHost();
  virtual uint16_t getRemotePort();
  virtual const void *getPostData(int &size);
  virtual Method getMethod();
  virtual std::string getHeader(const char *name);
  virtual void getHeaders(HeaderMap &headers);
  virtual void addHeaderImpl(const char *name, const char *value);
  virtual void removeHeaderImpl(const char *name);
  virtual void sendImpl(const void *data, int size, int code,
                        bool chunked);
  virtual void onSendEndImpl();
  virtual bool isUploadedFile(const String& filename);
  virtual bool moveUploadedFile(const String& filename,
    const String& destination);
  virtual bool getFiles(string &files);

  // task interface
  bool isDone();

  void addToPipeline(const string &s);

  bool isPipelineEmpty();

  String getResults(
    Array &headers,
    int &code,
    int64_t timeout_ms
  );

  bool getResults(
    Array &results,
    PageletServerTaskEvent* next_event
  );

  // ref counting
  void incRefCount();
  void decRefCount();

  const timespec& getStartTimer() const;
  int getTimeoutSeconds() const;

  void setAsioEvent(PageletServerTaskEvent *event);

private:
  std::atomic<int> m_refCount;
  int m_timeoutSeconds;

  string m_url;
  HeaderMap m_requestHeaders;
  bool m_get;
  string m_postData;
  string m_remoteHost;

  bool m_done;
  HeaderMap m_responseHeaders;
  string m_response;
  int m_code;

  std::deque<string> m_pipeline; // the intermediate pagelet results
  std::set<string> m_rfc1867UploadedFiles;
  string m_files; // serialized to use as $_FILES

  PageletServerTaskEvent *m_event;
};

class PageletServerTaskEvent : public AsioExternalThreadEvent {
public:

  ~PageletServerTaskEvent() {
    if (m_job) m_job->decRefCount();
  }

  void finish() {
    markAsFinished();
  }

  void setJob(PageletTransport *job) {
    job->incRefCount();
    m_job = job;
  }

protected:

  void unserialize(Cell& result) const {
    // Main string responses from pagelet thread.
    Array responses = Array::Create();

    // Create an event for the next results that might be used.
    PageletServerTaskEvent *event = new PageletServerTaskEvent();

    // Fetch all results from the transport that are currently available.
    bool done = m_job->getResults(responses, event);

    // Returned tuple/array.
    Array ret = Array::Create();
    ret.append(responses);

    if (done) {
      // If the whole thing is done, then we don't need a next event.
      event->abandon();
      ret.append(init_null_variant);
    } else {
      // The event was added to the job to be triggered next.
      ret.append(event->getWaitHandle());
    }

    cellDup(*(Variant(ret)).asCell(), result);
  }

private:
  PageletTransport* m_job;
  // string m_response;
  // Object m_next_wait_handle;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_PAGELET_SERVER_H_
