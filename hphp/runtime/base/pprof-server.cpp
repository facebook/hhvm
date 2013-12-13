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

#include "hphp/runtime/base/pprof-server.h"
#include "hphp/util/current-executable.h"
#include "hphp/util/logger.h"

#include "folly/Format.h"
#include "folly/Conv.h"

#include <mutex>
#include <condition_variable>

namespace HPHP {

namespace {
std::mutex s_clientMutex;
std::condition_variable s_clientWaitq;
bool s_cond = false;
}

void HeapProfileRequestHandler::handleRequest(Transport *transport) {
  const char *url = transport->getCommand().c_str();
  if (!strcmp(url, "hhprof/start")) {
    // if we can place the request, send a 200
    if (handleStartRequest(transport)) {
      transport->sendString("OK\n", 200);
    } else {
      transport->sendString("Resource Unavailable\n", 503);
    }
  } else if (!strcmp(url, "pprof/cmdline")) {
    // the first thing pprof does when you call the script is find out
    // the name of the binary running on the remote server
    transport->sendString(current_executable_path(), 200);
  } else if (!strcmp(url, "pprof/heap")) {
    // the next thing pprof does is hit this endpoint and get a profile
    // dump
    ProfileDump dump = ProfileController::waitForProfile();
    transport->sendString(dump.toPProfFormat(), 200);
  } else if (!strcmp(url, "pprof/symbol")) {
    // lastly, pprof hits this endpoint three times. the first time, it
    // hits with a HEAD request, which gives it some knowledge as to the
    // presence of the endpoint. then it hits with a GET request, and
    // expects the number of defined symbols in response. finally, it
    // hits with a POST request, with the POST data being a plus-separated
    // list of addresses for which it wants symbols
    if (transport->getMethod() == Transport::Method::HEAD) {
      transport->sendString("OK\n", 200);
    } else if (transport->getMethod() == Transport::Method::GET) {
      // actual number of sumbols is not really relevant
      // from the pprof documentation, pprof only considers values
      // that are either zero or non-zero
      transport->sendString("num_symbols: 1\n", 200);
    } else if (transport->getMethod() == Transport::Method::POST) {
      // split the post data by '+' character and resolve the symbol
      // for each
      int size;
      auto data = static_cast<const char *>(transport->getPostData(size));
      std::string res;

      std::vector<folly::StringPiece> addrs;
      folly::split('+', folly::StringPiece(data, size), addrs);
      for (const auto &addr : addrs) {
        // for each address we get from pprof, it expects a line formatted
        // like the following
        // <address>\t<symbol name>
        SrcKey sk = SrcKey::fromAtomicInt(
          static_cast<uint64_t>(std::stoll(addr.data(), 0, 16))
        );
        folly::toAppend(addr, "\t", sk.getSymbol(), "\n", &res);
      }
      transport->sendString(res, 200);

      if (RuntimeOption::ClientExecutionMode() &&
          RuntimeOption::HHProfServerProfileClientMode) {
        std::unique_lock<std::mutex> lock(s_clientMutex);
        s_cond = true;
        s_clientWaitq.notify_all();
      }
    }
  } else if (!strcmp(url, "hhprof/stop")) {
    // user has requested cancellation of the current profile dump
    ProfileController::cancelRequest();
    transport->sendString("OK\n", 200);
  } else {
    // the pprof server doesn't understand any other endpoints so just error
    // out
    Logger::Warning(folly::format(
      "Unknown HHProf endpoint requested, command was: {}", url
    ).str());
    transport->sendString("Not Found\n", 404);
  }
}

void HeapProfileRequestHandler::abortRequest(Transport *transport) {
  transport->sendString("Service Unavailable", 503);
}

// static
void HeapProfileServer::waitForPProf() {
  std::unique_lock<std::mutex> lock(s_clientMutex);

  s_clientWaitq.wait(lock, [&] { return s_cond; });
}

bool HeapProfileRequestHandler::handleStartRequest(Transport *transport) {
  // this endpoint is used to start profiling. if there is no profile
  // in progess, we start profiling, otherwise we error out
  std::string requestType = transport->getParam("type");
  std::string profileType = transport->getParam("profileType");
  ProfileType pt;

  if (profileType == "" || profileType == "default") {
    pt = ProfileType::Default;
  } else if (profileType == "heap") {
    pt = ProfileType::Heap;
  } else if (profileType == "allocation") {
    pt = ProfileType::Allocation;
  } else {
    // what are they even trying to do???
    Logger::Warning(folly::format(
      "Bad request received at HHProf start endpoint, profile type was: {}",
      profileType
    ).str());
    return false;
  }

  if (requestType == "" || requestType == "next") {
    // profile the next request, which is also the default
    // if a url was supplied, use that
    std::string url = transport->getParam("url");
    if (url == "") {
      return ProfileController::requestNext(pt);
    } else {
      return ProfileController::requestNextURL(pt, url);
    }
  } else if (requestType == "global") {
    // profile all requests until pprof attacks
    return ProfileController::requestGlobal(pt);
  } else {
    // what are they even trying to do???
    Logger::Warning(folly::format(
      "Bad request received at HHProf start endpoint, type was: {}",
      requestType
    ).str());
    return false;
  }
}

HeapProfileServerPtr HeapProfileServer::Server;

}
