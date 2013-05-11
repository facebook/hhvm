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

#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class PageletServer {
public:
  static bool Enabled();
  static void Restart();

  /**
   * Create a task. This returns a task handle, or null object
   * if there are no worker threads.
   */
  static Object TaskStart(CStrRef url, CArrRef headers,
                          CStrRef remote_host,
                          CStrRef post_data = null_string,
                          CArrRef files = null_array);

  /**
   * Query if a task is finished. This is non-blocking and can be called as
   * many times as desired.
   */
  static int64_t TaskStatus(CObjRef task);

  /**
   * Get results of a task. This is blocking until task is finished or times
   * out. The status code is set to -1 in the event of a timeout.
   */
  static String TaskResult(CObjRef task,
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

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_PAGELET_SERVER_H_
