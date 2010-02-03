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

#ifndef __SERVICE_THREAD_H__
#define __SERVICE_THREAD_H__

#include <util/async_func.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ServiceThread);
class ServiceThread : public AsyncFunc<ServiceThread>, public Synchronizable {
public:
  static ServiceThread *GetThisThread();

public:
  ServiceThread(const std::string &url);

  void threadRun();
  void waitForStarted();
  void notifyStarted();

private:
  bool m_started;
  std::string m_url;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __SERVICE_THREAD_H__
