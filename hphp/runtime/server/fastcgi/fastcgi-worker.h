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

#ifndef incl_HPHP_RUNTIME_SERVER_FASTCGI_FASTCGI_WORKER_H_
#define incl_HPHP_RUNTIME_SERVER_FASTCGI_FASTCGI_WORKER_H_

#include "hphp/runtime/server/fastcgi/fastcgi-transport.h"
#include "hphp/runtime/server/server-worker.h"
#include "hphp/runtime/server/job-queue-vm-stack.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

class FastCGIServer;
class FastCGITransportTraits;

DECLARE_BOOST_TYPES(FastCGIJob);
class FastCGIJob : public ServerJob {
friend class FastCGITransportTraits;
public:
  explicit FastCGIJob(std::shared_ptr<FastCGITransport> transport)
    : m_transport(transport) {}
  ~FastCGIJob() {}

  Transport* getTransport();

  void getRequestStart(struct timespec *outReqStart);

private:
  std::shared_ptr<FastCGITransport> m_transport;
  struct timespec reqStart;
};

class FastCGITransportTraits {
public:
  FastCGITransportTraits(FastCGIJobPtr job, void* context, int id);
  ~FastCGITransportTraits();

  Server *getServer() const;
  Transport *getTransport() const;

private:
  FastCGIServer *m_server;
  std::shared_ptr<FastCGITransport> m_transport;
};

typedef ServerWorker<FastCGIJobPtr,
                     FastCGITransportTraits> FastCGIWorker;

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_HTTP_SERVER_FASTCGI_FASTCGI_WORKER_H_

