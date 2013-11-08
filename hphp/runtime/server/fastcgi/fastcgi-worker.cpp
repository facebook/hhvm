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

#include "hphp/runtime/server/fastcgi/fastcgi-worker.h"
#include "hphp/runtime/server/fastcgi/fastcgi-transport.h"
#include "hphp/runtime/server/fastcgi/fastcgi-server.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

Transport* FastCGIJob::getTransport() {
  return m_transport.get();
}

void FastCGIJob::getRequestStart(struct timespec *outReqStart) {
  *outReqStart = reqStart;
}

///////////////////////////////////////////////////////////////////////////////

FastCGITransportTraits::FastCGITransportTraits(FastCGIJobPtr job,
                                               void* context,
                                               int id)
  : m_server((FastCGIServer*) context),
    m_transport(job->m_transport) {}

FastCGITransportTraits::~FastCGITransportTraits() {}

Server* FastCGITransportTraits::getServer() const {
  return m_server;
}

Transport* FastCGITransportTraits::getTransport() const {
  return m_transport.get();
}

///////////////////////////////////////////////////////////////////////////////
}

