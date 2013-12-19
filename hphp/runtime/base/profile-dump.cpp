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

#include "hphp/runtime/base/profile-dump.h"

#include "folly/Format.h"
#include "folly/Conv.h"
#include "hphp/runtime/base/execution-context.h"

namespace HPHP {

std::string ProfileDump::toPProfFormat() const {
  size_t currentCountSum = 0, currentBytesSum = 0;
  size_t accumCountSum   = 0, accumBytesSum   = 0;

  int numDumps = std::max(m_numDumps, 1);

  // aggregate for totals at top
  for (const auto &current : m_currentlyAllocated) {
    currentCountSum += current.second.m_count;
    currentBytesSum += current.second.m_bytes;
  }
  for (const auto &accum : m_accumAllocated) {
    accumCountSum += accum.second.m_count;
    accumBytesSum += accum.second.m_bytes;
  }

  std::string res;
  // print header, with the following format
  // heap profile: curobjs: curbytes [accumobjs: accumbytes] @ heapprofile
  folly::toAppend(
    folly::format(
      "heap profile: {}: {} [{}: {}] @ heapprofile\n",
      currentCountSum, currentBytesSum, accumCountSum, accumBytesSum
    ).str(), &res
  );

  // print information for allocations we have recorded for each call stack
  // each line has the following format
  // curobjs: curbytes [accumobjs: accumbytes] stacktrace...
  for (const auto &accum : m_accumAllocated) {
    const auto &trace = accum.first;
    // skip this information if we have a zero-length stack trace, because
    // that means we allocated this outside of PHP userland.
    if (trace.size() == 0) continue;
    // if the filters are set such that the number of bytes or allocations
    // at this site is not enough, we are going to filter it out
    if ((accum.second.m_count / numDumps <
         RuntimeOption::HHProfServerFilterMinAllocPerReq) ||
        (accum.second.m_bytes / numDumps <
         RuntimeOption::HHProfServerFilterMinBytesPerReq)) continue;
    // get current allocation count/bytes for the current stack trace. we
    // know this is present in the current allocations map because we did
    // insert it at the same time we inserted something into the cumulative
    // allocations map
    const auto &it = m_currentlyAllocated.find(trace);
    assert(it != m_currentlyAllocated.end());
    // dump current and cumulative count/bytes
    folly::toAppend(
      folly::format(
        "{}: {} [{}: {}] @",
        it->second.m_count, it->second.m_bytes,
        accum.second.m_count, accum.second.m_bytes
      ).str(), &res
    );
    // walk stack trace and pack srckeys into 64-bit ints so
    // they look like addresses
    for (const auto &sk : trace) {
      folly::toAppend(folly::format(" {:#x}", sk.toAtomicInt()), &res);
    }
    folly::toAppend("\n", &res);
  }

  // dump maps from /proc/pid/maps. we aren't going to use them because
  // we are going to manually resolve symbols ourselves later, and the
  // addresses we are dumping as part of the stack trace aren't even real
  // addresses anyway
  size_t buflen = 64;
  folly::toAppend("\nMAPPED_LIBRARIES:\n", &res);
  char buf[buflen];
  snprintf(buf, buflen, "/proc/%d/maps", getpid());
  FILE *f = fopen(buf, "r");
  size_t bytesRead;
  while ((bytesRead = fread(buf, 1, buflen, f)) > 0) {
    folly::toAppend(folly::StringPiece(buf, bytesRead), &res);
  }
  fclose(f);
  return res;
}

// ProfileController state
namespace {
// type of current request
// Next: profile the next request
// NextURL: profile the next request to m_url
// Global: profile all requests until stopped
enum RequestType {
  None,
  Next,
  NextURL,
  Global
} m_reqType;

// url to profile for NextURL type requests
std::string m_url;

// currently-held dump, valid if we are complete
ProfileDump m_dump;

// state of the controller
// Waiting: no request is active
// Pending: a request is active, but has not yet
//   been fulfilled
// Complete: a request is active, and has been
//   fulfilled
enum State {
  Waiting,
  Pending,
  Complete
} m_state;

// Profile type (heap vs. allocation)
ProfileType m_profileType = ProfileType::Default;

// synchronization primitives
std::condition_variable m_waitq;
std::mutex m_mutex;

};

// static
bool ProfileController::requestNext(ProfileType type) {
  std::unique_lock<std::mutex> lock(m_mutex);

  // don't clobber another request!
  if (m_state != State::Waiting) return false;
  // we have the mutex and no other request is pending, we can
  // place the request
  m_reqType = RequestType::Next;
  m_state = State::Pending;
  m_profileType = type;

  return true;
}

// static
bool ProfileController::requestNextURL(ProfileType type,
                                       const std::string &url) {
  std::unique_lock<std::mutex> lock(m_mutex);

  if (m_state != State::Waiting) return false;
  m_reqType = RequestType::NextURL;
  m_url = url;
  m_state = State::Pending;
  m_profileType = type;

  return true;
}

// static
bool ProfileController::requestGlobal(ProfileType type) {
  std::unique_lock<std::mutex> lock(m_mutex);

  if (m_state != State::Waiting) return false;
  m_reqType = RequestType::Global;
  m_state = State::Pending;
  m_profileType = type;

  // clean up the dump since we are going to copy in data
  // from other dumps when they offer theirs
  m_dump.clear();

  return true;
}

// static
void ProfileController::cancelRequest() {
  std::unique_lock<std::mutex> lock(m_mutex);

  // changing the state is enough to cancel the currently
  // active request; no VM thread will put their dump here
  // if the state is waiting
  m_state = State::Waiting;
  m_waitq.notify_all();
}

// static
void ProfileController::offerProfile(const ProfileDump &dump) {
  std::unique_lock<std::mutex> lock(m_mutex);

  // we have to be waiting for a profile, or it has to be
  // a global profile (since we merge many profiles into
  // a global request)
  if (m_state != State::Pending &&
      !(m_reqType == RequestType::Global &&
        m_state == State::Complete)) {
    return;
  }

  // check if we are fulfilling the pending request
  // 1. if the type is Next, we always fill it
  // 2. if the type is NextURL, we fill it if the
  //    URL matches the one the VM thread is running
  // 3. if the type is Global, we add our dump info
  //    to the dump in the controller
  switch (m_reqType) {
    case RequestType::Next:
      m_dump = dump;
      break;
    case RequestType::NextURL:
      if (g_context->getTransport()->getCommand() != m_url) return;
      m_dump = dump;
      break;
    case RequestType::Global:
      m_dump += dump;
      break;
    default:
      not_reached();
  }
  m_state = State::Complete;
  m_waitq.notify_all();
}

// static
ProfileDump ProfileController::waitForProfile() {
  std::unique_lock<std::mutex> lock(m_mutex);

  auto cond = [&] { return m_state != State::Pending; };
  if (RuntimeOption::ClientExecutionMode() &&
      RuntimeOption::HHProfServerProfileClientMode) {
    m_waitq.wait(lock, cond);
  } else {
    m_waitq.wait_for(
      lock,
      std::chrono::seconds(RuntimeOption::HHProfServerTimeoutSeconds),
      cond
    );
  }

  // check to see if someone else grabbed the profile
  if (m_state == State::Waiting) return ProfileDump();
  // otherwise, the profile is ours. reset the state and return it
  m_state = State::Waiting;
  m_reqType = RequestType::None;
  return m_dump;
}

ProfileType ProfileController::profileType() {
  return m_profileType;
}

}
