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

#ifndef incl_HPHP_SERVICE_THREAD_H_
#define incl_HPHP_SERVICE_THREAD_H_

#include "hphp/util/async-func.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ServiceThread);
class ServiceThread : public AsyncFunc<ServiceThread>, public Synchronizable {
public:
  static bool IsServiceThread();
  static ServiceThread *GetThisThread();

public:
  explicit ServiceThread(const std::string &url, bool loop = false);

  void threadRun();
  void waitForStarted();
  void notifyStarted();
  bool waitForStopped(int seconds);
  void notifyStopped();

private:
  bool m_loop;
  bool m_started;
  bool m_stopped;
  std::string m_url;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_SERVICE_THREAD_H_
