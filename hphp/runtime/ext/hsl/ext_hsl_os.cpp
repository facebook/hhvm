/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/file-await.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/server/cli-server-ext.h"
#include "hphp/system/systemlib.h"

#include <fcntl.h>
#include <stdio.h>

namespace HPHP {
namespace {

const StaticString
  s_HSLFileDescriptor("HSLFileDescriptor"),
  s_fd("fd"),
  s_ErrnoException("HH\\Lib\\_Private\\_OS\\ErrnoException"),
  s_FQHSLFileDescriptor("HH\\Lib\\OS\\FileDescriptor");

Class* s_FileDescriptorClass = nullptr;

IMPLEMENT_REQUEST_LOCAL(std::set<int>, s_fds_to_close);

[[noreturn]]
void throw_errno_exception(int number) {
  throw_object(
    s_ErrnoException,
    make_vec_array(
      folly::sformat("Errno {}: {}", number, ::strerror(number)),
      number
    )
  );
}

template<class T>
void throw_errno_if_minus_one(T var) {
  if (var == -1) {
    throw_errno_exception(errno);
  }
}

} // namespace

struct HSLFileDescriptor {
  static Object newInstance(int fd) {
    assertx(s_FileDescriptorClass);
    Object obj { s_FileDescriptorClass };

    auto* data = Native::data<HSLFileDescriptor>(obj);
    data->m_fd = fd;

    s_fds_to_close->insert(fd);
    return obj;
  }

  static HSLFileDescriptor* get(const Object& obj) {
    assertx(!obj.isNull());
    assertx(obj->instanceof(s_FQHSLFileDescriptor));
    return Native::data<HSLFileDescriptor>(obj);
  }

  static int fd(const Object& obj) {
    return get(obj)->fd();
  }

  int fd() const {
    if (m_fd < 0) throw_errno_exception(EBADF);
    return m_fd;
  }

  void close() {
    int result = ::close(fd());
    throw_errno_if_minus_one(result);
    s_fds_to_close->erase(m_fd);
    m_fd = -1;
  }

  Array __debugInfo() const {
    return make_darray(
      s_fd, VarNR{make_tv<KindOfInt64>(m_fd)}
    );
  }

 private:
   // intentionally not closed by destructor: that would introduce observable
   // refcounting behavior. Instead, it's closed at end of request from
   // s_fds_to_close.
   int m_fd;
};

Array HHVM_METHOD(HSLFileDescriptor, __debugInfo) {
  return Native::data<HSLFileDescriptor>(this_)->__debugInfo();
}

namespace {

template<class T>
T hsl_cli_unwrap(CLISrvResult<T, int> res) {
  if (res.succeeded()) {
    return res.result();
  }
  throw_errno_exception(res.error());
}

CLISrvResult<FdData, int> CLI_CLIENT_HANDLER(HSL_os_open, std::string path, int64_t flags, int64_t mode) {
  auto fd = ::open(path.c_str(), flags, mode);
  if (fd == -1) {
    return { CLIError {}, errno };
  }
  return { CLISuccess {}, FdData { fd } };
}

Object HHVM_FUNCTION(HSL_os_open, const String& path, int64_t flags, int64_t mode) {
  int fd = hsl_cli_unwrap(INVOKE_ON_CLI_CLIENT(
    HSL_os_open,
    path.toCppString(),
    flags,
    mode
  )).fd;
  assertx(fd >= 0);
  return HSLFileDescriptor::newInstance(fd);
}

int64_t HHVM_FUNCTION(HSL_os_write, const Object& obj, const String& data) {
  auto fd = HSLFileDescriptor::fd(obj);
  ssize_t written = ::write(fd, data.data(), data.length());
  throw_errno_if_minus_one(written);
  return written;
}

void HHVM_FUNCTION(HSL_os_close, const Object& obj) {
  HSLFileDescriptor::get(obj)->close();
}

Array HHVM_FUNCTION(HSL_os_pipe) {
  int fds[2];
  throw_errno_if_minus_one(::pipe(fds));
  return make_varray(
    HSLFileDescriptor::newInstance(fds[0]),
    HSLFileDescriptor::newInstance(fds[1])
  );
}

Object HHVM_FUNCTION(HSL_os_poll_async,
                     const Object& fd_wrapper,
                     int64_t events,
                     int64_t timeout_ns) {
  if (!(events & FileEventHandler::READ_WRITE)) {
    SystemLib::throwExceptionObject("Must poll for read, write, or both");
  }
  if (timeout_ns< 0) {
    SystemLib::throwExceptionObject("Poll timeout must be >= 0");
  }
  auto fd = HSLFileDescriptor::fd(fd_wrapper);

  const auto originalFlags = ::fcntl(fd, F_GETFL);
  // This always succeeds...
  ::fcntl(fd, F_SETFL, originalFlags | O_ASYNC);
  // ... but sometimes doesn't actually do anything
  const bool isAsyncableFd = ::fcntl(fd, F_GETFL) & O_ASYNC;
  ::fcntl(fd, F_SETFL, originalFlags);
  if (!isAsyncableFd) {
    throw_errno_exception(ENOTSUP);
  }

  auto ev = new FileAwait(
    fd,
    events,
    std::chrono::nanoseconds(timeout_ns)
  );
  try {
    return Object{ev->getWaitHandle()};
  } catch (...) {
    assertx(false);
    ev->abandon();
    throw;
  }
}

struct OSExtension final : Extension {

  OSExtension() : Extension("hsl_os", "0.1") {}

  void moduleInit() override {
    // Remember to update the HHI :)
    // open() flags ----------
    // The preprocessor doesn't like "\" immediately before a ##
#define O_(name) HHVM_RC_INT(HH\\Lib\\OS\\O_##name, O_##name)
    O_(RDONLY);
    O_(WRONLY);
    O_(RDWR);
    O_(NONBLOCK);
    O_(APPEND);
    O_(CREAT);
    O_(TRUNC);
    O_(EXCL);
    O_(NOFOLLOW);
    O_(CLOEXEC);
#undef O_
    // MacOS: O_EVTONLY, O_SHLOCK, O_EXLOCK, O_SYMLINK
    // Linux: ... lots ...

    HHVM_FALIAS(HH\\Lib\\_Private\\_OS\\open, HSL_os_open);
    HHVM_FALIAS(HH\\Lib\\_Private\\_OS\\pipe, HSL_os_pipe);
    HHVM_FALIAS(HH\\Lib\\_Private\\_OS\\poll_async, HSL_os_poll_async);
    HHVM_FALIAS(HH\\Lib\\_Private\\_OS\\write, HSL_os_write);
    HHVM_FALIAS(HH\\Lib\\_Private\\_OS\\close, HSL_os_close);

    HHVM_NAMED_ME(HH\\Lib\\OS\\FileDescriptor, __debugInfo, HHVM_MN(HSLFileDescriptor, __debugInfo));

    CLI_REGISTER_HANDLER(HSL_os_open);
    Native::registerNativeDataInfo<HSLFileDescriptor>(s_HSLFileDescriptor.get());
    loadSystemlib();
    s_FileDescriptorClass = Unit::lookupClass(s_FQHSLFileDescriptor.get());
    assertx(s_FileDescriptorClass);
  }

  void requestShutdown() override {
    if (s_fds_to_close.isNull()) {
      return;
    }
    for (int fd : *s_fds_to_close) {
      // retrying this on EINTR would be unsafe: the call can be interrupted
      // after the FD has been freed but the kernel is doing other work - and
      // the FD may have been reused by the time we retry, so retrying on
      // EINTR may close some unrelated fd
      ::close(fd);
    }
    s_fds_to_close.destroy();
  }
} s_os_extension;

} // anonymous namespace
} // namespace HPHP
