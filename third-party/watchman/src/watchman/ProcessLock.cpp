/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ProcessLock.h"

#include <fmt/core.h>
#include <folly/String.h>

#include "watchman/Logging.h"

#ifndef _WIN32
#include <sys/file.h>
#endif

namespace watchman {

ProcessLock ProcessLock::acquire(const std::string& pid_file) {
  auto result = tryAcquire(pid_file);
  if (auto* error = std::get_if<std::string>(&result)) {
    log(ERR, *error, "\n");
    exit(1);
  }
  return std::move(std::get<ProcessLock>(result));
}

std::variant<ProcessLock, ProcessLock::LockError> ProcessLock::tryAcquire(
    const std::string& pid_file) {
#ifndef _WIN32
  FileDescriptor fd(
      open(pid_file.c_str(), O_RDWR | O_CREAT, 0644),
      FileDescriptor::FDType::Generic);

  if (!fd) {
    return fmt::format(
        "Failed to open pidfile {} for write: {}",
        pid_file,
        folly::errnoStr(errno));
  }
  // Ensure that no children inherit the locked pidfile descriptor
  fd.setCloExec();

  // Watchman only starts its server when it's considered not running. But it's
  // possible the old Watchman server has just shut down, and the lock isn't
  // released yet. If we fail to acquire the lock, return, and let the caller
  // decide whether to retry.

  // Use flock because it transfers the lock to child processes through
  // fork().
  int result = ::flock(fd.fd(), LOCK_EX | LOCK_NB);
  const int errno_copy = errno;
  if (result != 0) {
    char pidstr[32];
    int len = read(fd.fd(), pidstr, sizeof(pidstr) - 1);
    pidstr[len] = '\0';

    return fmt::format(
        "Failed to lock pidfile {}: process {} owns it: {}, and my pid = {}",
        pid_file,
        pidstr,
        folly::errnoStr(errno_copy),
        getpid());
  }

  return ProcessLock{std::move(fd)};
#else
  // One does not simply, and without risk of races, write a pidfile
  // on win32.  Instead we're using a named mutex in the global namespace.
  // This gives us a very simple way to exclusively claim ownership of
  // the lock for this user.  To make things a little more complicated,
  // since we scope our locks based on the state dir location and require
  // this to work for our integration tests, we need to create a unique
  // name per state dir.  This is made even more interesting because
  // we are forbidden from using windows directory separator characters
  // in the name, so we cannot simply concatenate the state dir path
  // with a watchman specific prefix.  Instead we iterate the path
  // and rewrite any backslashes with forward slashes and use that
  // for the name.
  // Using a mutex for this does make it more awkward to discover
  // the process id of the exclusive owner, but that's not critically
  // important; it is possible to connect to the instance and issue
  // a get-pid command if that is needed.

  // We use the global namespace so that we ensure that we have one
  // watchman process per user per state dir location.  If we didn't
  // use the Global namespace we'd end using a local namespace scoped
  // to the user session and that might cause confusion/insanity if
  // they are doing something elaborate like being logged in via
  // ssh in multiple sessions and expecting to share state.
  std::string name("Global\\Watchman-");
  for (const auto& it : pid_file) {
    if (it == '\\') {
      // We're not allowed to use backslash in the name, so normalize
      // to forward slashes.
      name.append("/");
    } else {
      name.push_back(it);
    }
  }

  HANDLE mutex = CreateMutexA(nullptr, true, name.c_str());

  if (!mutex) {
    // Treat unexpected errors as fatal.
    log(ERR,
        "Failed to create mutex named: ",
        name,
        ": ",
        GetLastError(),
        "\n");
    exit(1);
  }

  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    // Allow retrying failure to acquire the lock.
    log(ERR,
        "Failed to acquire mutex named: ",
        name,
        "; watchman is already running for this context\n");
    exit(1);
  }

  /* We are intentionally not closing the mutex and intentionally not storing
   * a reference to it anywhere: the intention is that it remain locked
   * for the rest of the lifetime of our process.
   * CloseHandle(mutex); // NOPE!
   */
  return ProcessLock{};
#endif
}

ProcessLock::Handle ProcessLock::writePid(const std::string& pid_file) {
#ifndef _WIN32
  CHECK(fd_) << "writePid may only be called after acquire";

  // Replace contents of the pidfile with our pid string
  if (0 == ftruncate(fd_.fd(), 0)) {
    pid_t mypid = getpid();
    auto pidString = fmt::to_string(mypid);
    ignore_result(write(fd_.fd(), pidString.data(), pidString.size()));
    fsync(fd_.fd());
  } else {
    log(ERR,
        "Failed to truncate pidfile ",
        pid_file,
        ": ",
        folly::errnoStr(errno),
        "\n");
  }

  /* We are intentionally not closing the fd and intentionally not storing
   * a reference to it anywhere: the intention is that it remain locked
   * for the rest of the lifetime of our process.
   * close(fd); // NOPE!
   */
  fd_.release();
#endif
  return Handle{};
}

} // namespace watchman
