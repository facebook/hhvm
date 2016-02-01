/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/hhprof.h"

#include <folly/File.h>
#include <folly/FileUtil.h>
#include <folly/Singleton.h>

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/util/current-executable.h"
#include "hphp/util/logger.h"
#include "hphp/util/stack-trace.h"

#include <sstream>
#include <string>

namespace HPHP {

TRACE_SET_MOD(hhprof);

IMPLEMENT_THREAD_LOCAL(HHProf::Request, HHProf::Request::s_request);

namespace { folly::Singleton<HHProf> s_hhprof; }
static std::shared_ptr<HHProf> hhprof() {
  // Centralize try_get() calls to document that despite the scary name, it
  // won't fail under any conditions we subject it to.
  std::shared_ptr<HHProf> h = s_hhprof.try_get();
  assert(h.get() != nullptr);
  return h;
}

///////////////////////////////////////////////////////////////////////////////
// HHProf::Request.

void HHProf::Request::setupImpl(Transport* transport) {
  m_active = hhprof()->setupRequest(transport);
  TRACE(1, "setup request (active:%s)\n", (m_active ? "true" : "false"));
}

void HHProf::Request::teardownImpl() {
  TRACE(1, "teardown request (active:%s)\n", (m_active ? "true" : "false"));
  if (!m_active) return;
  hhprof()->teardownRequest();
}

void HHProf::Request::startProfilingImpl() {
  TRACE(1, "start profiling (active:%s)\n", (m_active ? "true" : "false"));
  if (!m_active) return;
  hhprof()->startProfiledRequest();
}

void HHProf::Request::finishProfilingImpl() {
  TRACE(1, "finish profiling (active:%s)\n", (m_active ? "true" : "false"));
  if (!m_active) return;
  hhprof()->finishProfiledRequest();
}

///////////////////////////////////////////////////////////////////////////////
// HHProf.

bool HHProf::reset(Transport* transport) {
  if (m_requestState == RequestState::Pending) {
    transport->sendString("Profile pending; try again", 503);
    return true;
  }

  size_t lg_sample;
  std::string lgSample = transport->getParam("lgSample");
  if (lgSample == "") {
    mallctlRead("prof.lg_sample", &lg_sample);
  } else {
    errno = 0;
    lg_sample = strtoul(lgSample.c_str(), nullptr, 0);
    if (lg_sample == ULONG_MAX && errno != 0) {
      transport->sendString(folly::format(
        "Invalid lg_sample {} for /hhprof/start request", lgSample).str(),
        400);
      return true;
    }
  }

  bool accum;
  std::string profileType = transport->getParam("profileType");
  if (profileType == "" || profileType == "current") {
    accum = false;
  } else if (profileType == "cumulative") {
    accum = true;
  } else {
    transport->sendString(folly::format(
      "Invalid profile type {} for /hhprof/start request", profileType).str(),
      400);
    return true;
  }

  // TODO(#8947159): Configure cumulative statistics during reset.
  if (false) {
    mallctlWrite("prof.accum_init", accum);
  }
  mallctlWrite("prof.reset", lg_sample);

  return false;
}

void HHProf::start(Transport* transport) {
  mallctlWrite("prof.active", true);

  std::string requestType = transport->getParam("requestType");
  if (requestType == "all") {
    m_requestType = RequestType::All;
  } else {
    if (requestType != "" && requestType != "next") {
      transport->sendString(folly::format(
        "Invalid request type {} for /hhprof/start request",
        requestType).str(), 400);
      return;
    }
    std::string url = transport->getParam("url");
    if (url == "") {
      m_requestType = RequestType::Next;
    } else {
      m_requestType = RequestType::NextURL;
      m_url = url;
    }
  }
  m_requestState = RequestState::Pending;

  transport->sendString(folly::format("{}\n", m_nextID).str());
}

void HHProf::handleHHProfStartImpl(Transport* transport) {
  if (!RuntimeOption::HHProfEnabled) {
    transport->sendString("Specify -vHHProf.Enabled=true", 503);
    return;
  }

  std::unique_lock<std::mutex> lock(m_mutex);

  if (reset(transport)) return;
  start(transport);
}

void HHProf::handleHHProfStatusImpl(Transport* transport) {
  std::unique_lock<std::mutex> lock(m_mutex);
  std::string requestTypeStr;
  switch (m_requestType) {
  case RequestType::None:    requestTypeStr = "None"; break;
  case RequestType::Next:    requestTypeStr = "Next"; break;
  case RequestType::NextURL: requestTypeStr = folly::format("NextURL ({})",
                                                            m_url).str(); break;
  case RequestType::All:     requestTypeStr = "All"; break;
  }
  std::string requestStateStr;
  switch (m_requestState) {
  case RequestState::None:     requestStateStr = "None"; break;
  case RequestState::Pending:  requestStateStr = "Pending"; break;
  case RequestState::Sampling: requestStateStr = "Sampling"; break;
  case RequestState::Dumped:   requestStateStr = "Dumped"; break;
  }
  bool active;
  mallctlRead("prof.active", &active);

  std::ostringstream res;
  res << "---" << std::endl;
  res << "HHProf.Enabled: " << (RuntimeOption::HHProfEnabled ? "true" : "false")
      << std::endl;
  res << "HHProf.Active: " << (RuntimeOption::HHProfActive ? "true" : "false")
      << std::endl;
  res << "HHProf.Accum: " << (RuntimeOption::HHProfAccum ? "true" : "false")
      << std::endl;
  res << "HHProf.Request: " << (RuntimeOption::HHProfRequest ? "true" : "false")
      << std::endl;
  res << "RequestType: " << requestTypeStr << std::endl;
  res << "RequestState: " << requestStateStr << std::endl;
  res << "DumpId: " << m_dumpResult.id() << std::endl;
  res << "prof.active: " << (active ? "true" : "false")
      << std::endl;
  res << "..." << std::endl;

  transport->sendString(res.str());
}

void HHProf::handleHHProfStopImpl(Transport* transport) {
  if (!RuntimeOption::HHProfEnabled) {
    transport->sendString("Specify -vHHProf.Enabled=true", 503);
    return;
  }

  std::unique_lock<std::mutex> lock(m_mutex);

  mallctlWrite("prof.active", false);

  m_dumpResult = captureDump();
  m_requestState = RequestState::Dumped;

  transport->sendString("OK\n");
}

HHProf::DumpResult HHProf::captureDump() {
  unsigned id = m_nextID++;
  // Create and open a tempfile, use the tempfile's name when dumping a heap
  // profile, then open the resulting file.  Take care to close both file
  // descriptors and unlink the output file after reading its contents.
  char tmpName[] = "/tmp/hhprof.XXXXXX";
  int tmpFd = mkstemp(tmpName);
  if (tmpFd == -1) {
    std::ostringstream estr;
    estr << "Error opening " << tmpName << std::endl;
    return HHProf::DumpResult("", id, 503, estr.str());
  }
  folly::File tmpFile(tmpFd, true);

  SCOPE_EXIT { unlink(tmpName); };

  int err = jemalloc_pprof_dump(tmpName, true);
  if (err != 0) {
    std::ostringstream estr;
    estr << "Error " << err << " dumping " << tmpName << std::endl;
    return HHProf::DumpResult("", id, 503, estr.str());
  }
  std::string dump;
  if (!folly::readFile(tmpName, dump)) {
    std::ostringstream estr;
    estr << "Error reading " << tmpName << std::endl;
    return HHProf::DumpResult("", id, 503, estr.str());
  }
  return HHProf::DumpResult(dump, id);
}

void HHProf::handlePProfHeapImpl(Transport* transport) {
  if (!RuntimeOption::HHProfEnabled) {
    transport->sendString("Specify -vHHProf.Enabled=true", 503);
    return;
  }

  std::unique_lock<std::mutex> lock(m_mutex);

  if (RuntimeOption::HHProfRequest) {
    switch (m_requestState) {
    case RequestState::None:
      captureDump().send(transport);
      break;
    case RequestState::Pending:
    case RequestState::Sampling:
      switch (m_requestType) {
      case RequestType::None:
      case RequestType::Next:
      case RequestType::NextURL:
        m_dumpResult.send(transport);
        break;
      case RequestType::All:
        captureDump().send(transport);
        break;
      }
      break;
    case RequestState::Dumped:
      m_dumpResult.send(transport);
      break;
    }
  } else {
    captureDump().send(transport);
  }
}

bool HHProf::shouldProfileRequest(Transport* transport) {
  bool canProfile =
#ifdef USE_JEMALLOC
    (mallctl != nullptr)
#else
    false
#endif
  ;

  switch (m_requestState) {
  case RequestState::None:
    return false;
  case RequestState::Pending:
    switch (m_requestType) {
    case RequestType::None:
      return false;
    case RequestType::Next:
      break;
    case RequestType::NextURL:
      if (transport->getCommand() != m_url) return false;
      break;
    case RequestType::All:
      break;
    }
    if (canProfile) {
      m_requestState = RequestState::Sampling;
    }
    return canProfile;
  case RequestState::Sampling:
    switch (m_requestType) {
    case RequestType::None:
    case RequestType::Next:
    case RequestType::NextURL:
      return false;
    case RequestType::All:
      return canProfile;
    }
    not_reached();
  case RequestState::Dumped:
    switch (m_requestType) {
    case RequestType::None:
    case RequestType::Next:
    case RequestType::NextURL:
      return false;
    case RequestType::All:
      return canProfile;
    }
    not_reached();
  }
  not_reached();
}

bool HHProf::setupRequest(Transport* transport) {
  std::unique_lock<std::mutex> lock(m_mutex);

  bool active;
  mallctlRead("prof.active", &active);
  if (!active) return false;

  if (!(HHProf::shouldProfileRequest(transport))) return false;
  MemoryManager::setupProfiling();
  return true;
}

void HHProf::teardownRequest() {
  MemoryManager::teardownProfiling();
}

void HHProf::startProfiledRequest() {
  mallctlWrite("thread.prof.active", true);
}

void HHProf::finishProfiledRequest() {
  std::unique_lock<std::mutex> lock(m_mutex);

  mallctlWrite("thread.prof.active", false);
  if (RuntimeOption::HHProfRequest && m_requestType != RequestType::All) {
    m_dumpResult = captureDump();
    m_requestState = RequestState::Dumped;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Exposed API, mainly static wrapper methods.

/* static */ void HHProf::Init() {
  if (!RuntimeOption::HHProfEnabled) return;

  // Ensure that profiling is enabled in jemalloc.
  bool opt_prof;
  mallctlRead("opt.prof", &opt_prof);
  if (!opt_prof) {
    const char* err = "HHProf incompatible with MALLOC_CONF=prof:false";
    Logger::Error("%s", err);
    std::cerr << err << std::endl;
    throw std::runtime_error(err);
  }

  mallctlWrite("prof.active", RuntimeOption::HHProfActive);

  bool prof_accum;
  mallctlRead("opt.prof_accum", &prof_accum);
  if (RuntimeOption::HHProfAccum != prof_accum) {
    // TODO(#8947159): Dynamically configure cumulative statistics rather than
    // forcing the decision at startup.
    const char* err = "HHProf.Accum requires MALLOC_CONF=prof_accum:true";
    Logger::Error("%s", err);
    std::cerr << err << std::endl;
    throw std::runtime_error(err);
  }

  // Configure thread.prof.active for the main thread.
  mallctlWrite("thread.prof.active", !RuntimeOption::HHProfRequest);
  // Configure prof.thread_active_init before launching additional threads.
  mallctlWrite("prof.thread_active_init", !RuntimeOption::HHProfRequest);
}

/* static */ void HHProf::HandleHHProfStart(Transport* transport) {
  hhprof()->handleHHProfStartImpl(transport);
}

/* static */ void HHProf::HandleHHProfStatus(Transport* transport) {
  hhprof()->handleHHProfStatusImpl(transport);
}

/* static */ void HHProf::HandleHHProfStop(Transport* transport) {
  hhprof()->handleHHProfStopImpl(transport);
}

/* static */ void HHProf::HandlePProfCmdline(Transport* transport) {
  if (!RuntimeOption::HHProfEnabled) {
    transport->sendString("Specify -vHHProf.Enabled=true", 503);
    return;
  }

  transport->sendString(current_executable_path());
}

/* static */ void HHProf::HandlePProfHeap(Transport* transport) {
  hhprof()->handlePProfHeapImpl(transport);
}

/* static */ void HHProf::HandlePProfSymbol(Transport* transport) {
  if (!RuntimeOption::HHProfEnabled) {
    transport->sendString("Specify -vHHProf.Enabled=true", 503);
    return;
  }

  switch (transport->getMethod()) {
  case Transport::Method::HEAD:
    // Useful only for detecting the presence of the pprof/symbol endpoint.
    transport->sendString("OK\n");
    break;
  case Transport::Method::GET:
    // Return non-zero to indicate symbol lookup is supported.
    transport->sendString("num_symbols: 1\n");
    break;
  case Transport::Method::POST: {
    // Split the '+'-delimited addresses and resolve their associated symbols.
    int size;
    auto data = static_cast<const char *>(transport->getPostData(size));
    std::string result;
    std::vector<folly::StringPiece> addrs;
    folly::split('+', folly::StringPiece(data, size), addrs);
    bool phpOnly = (transport->getParam("retain") == "^PHP::");
    // For each address append a line of the form:
    //   <addr>\t<symbol>
    StackTrace::PerfMap pm;
    for (const auto &addr : addrs) {
      if (!addr.size()) {
        continue;
      }
      std::string sval(addr.data(), addr.size());
      void* pval = (void*)std::stoull(sval, 0, 16);
      if (phpOnly && !jit::mcg->isValidCodeAddress(jit::TCA(pval))) {
        continue;
      }
      std::shared_ptr<StackTrace::Frame> frame = StackTrace::Translate(pval,
                                                                       &pm);
      if (frame->funcname != "TC?") {
        folly::toAppend(addr, "\t", frame->funcname, "\n", &result);
      } else if (phpOnly || jit::mcg->isValidCodeAddress(jit::TCA(pval))) {
        // Prefix address such that it can be distinguished as residing within
        // an unresolved PHP function.
        folly::toAppend(addr, "\tPHP::", sval, "\n", &result);
      } else {
        folly::toAppend(addr, "\t", sval, "\n", &result);
      }
    }
    transport->sendString(result);
    break;
  }
  default:
    transport->sendString("Unsupported method", 503);
    break;
  }
}

}
