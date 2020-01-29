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
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/system/systemlib.h"

#include <fcntl.h>
#include <stdio.h>

// TODO: Add _Private\ErrnoException(int), throw that instead
#define THROW_AS_ERRNO(code) throw_errno_exception(code)
#define THROW_ERRNO_IF_MINUS_ONE(var) if (var == -1) { THROW_AS_ERRNO(errno); }

namespace HPHP {
namespace {

const StaticString s_HSLFileDescriptor("HSLFileDescriptor");
const StaticString s_fd("fd");
const StaticString s_valid("valid");
const StaticString s_source("source");
const StaticString s_ErrnoException("HH\\Lib\\_Private\\OS\\ErrnoException");

IMPLEMENT_REQUEST_LOCAL(std::set<int>, s_fds_to_close);

[[noreturn]]
void throw_errno_exception(int number) {
  Array params;
  params.append(folly::sformat("Errno {}: {}", number, ::strerror(number)));
  params.append(number);
  throw_object(s_ErrnoException, params);
}

struct HSLFileDescriptor {
  static Object newInstance(int fd, const String& source) {
    if (s_class == nullptr) {
      s_class = Unit::lookupClass(s_classname.get());
    }
    assertx(s_class);
    Object obj { s_class };

    auto* data = Native::data<HSLFileDescriptor>(obj);
    data->m_fd = fd;
    data->m_valid = true;
    data->m_source = source;

    s_fds_to_close->insert(fd);
    return obj;
  }

  int fd() const {
    if (!m_valid) {
      THROW_AS_ERRNO(EBADF);
    }
    return m_fd;
  }

  void close() {
    if (!m_valid) {
      THROW_AS_ERRNO(EBADF);
    }
    int result = ::close(m_fd);
    THROW_ERRNO_IF_MINUS_ONE(result);
    s_fds_to_close->erase(m_fd);
    m_valid = false;
  }

  Array __debugInfo() const {
    Array ret = Array::CreateDArray();
    ret.set(s_fd, m_fd);
    ret.set(s_valid, m_valid);
    ret.set(s_source, m_source);
    return ret;
  }

  private:
    static Class* s_class;
    static StaticString s_classname;
    int m_fd;
    bool m_valid;
    String m_source;
};

Class* HSLFileDescriptor::s_class = nullptr;
StaticString HSLFileDescriptor::s_classname("HH\\Lib\\OS\\FileDescriptor");

Array HHVM_METHOD(HSLFileDescriptor, __debugInfo) {
  return Native::data<HSLFileDescriptor>(this_)->__debugInfo();
}

Object HHVM_FUNCTION(HSL_os_open, const String& path, int64_t flags, const Variant& mode) {
int fd;
  if (flags & O_CREAT) {
    // TODO: throw ??? if mode is not an int
    fd = ::open(path.c_str(), flags, mode.asInt64Val());
  } else {
    fd = ::open(path.c_str(), flags);
  }
  THROW_ERRNO_IF_MINUS_ONE(fd);
  return HSLFileDescriptor::newInstance(fd, folly::sformat("open('{}', {})", path.c_str(), flags));
}

int64_t HHVM_FUNCTION(HSL_os_write, const Object& obj, const String& data) {
  auto fd = Native::data<HSLFileDescriptor>(obj)->fd();
  ssize_t written = ::write(fd, data.data(), data.length());
  THROW_ERRNO_IF_MINUS_ONE(written);
  return written;
}

void HHVM_FUNCTION(HSL_os_close, const Object& obj) {
   Native::data<HSLFileDescriptor>(obj)->close();
}

struct OSExtension final : Extension {

  OSExtension() : Extension("hsl_os", "0.1") {}

  void moduleInit() override {
    Native::registerNativeDataInfo<HSLFileDescriptor>(s_HSLFileDescriptor.get());

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
    O_(SHLOCK);
    O_(EXLOCK);
    O_(NOFOLLOW);
    O_(SYMLINK);
    O_(CLOEXEC);
#undef O_
    // MacOS: O_EVTONLY
    // Linux: ... lots ...

    HHVM_FALIAS(HH\\Lib\\_Private\\OS\\open, HSL_os_open);
    HHVM_FALIAS(HH\\Lib\\_Private\\OS\\write, HSL_os_write);
    HHVM_FALIAS(HH\\Lib\\_Private\\OS\\close, HSL_os_close);

    HHVM_NAMED_ME(HH\\Lib\\OS\\FileDescriptor, __debugInfo, HHVM_MN(HSLFileDescriptor, __debugInfo));

    loadSystemlib();
  }

  void requestShutdown() override {
    int result = 0;
    for (int fd : *s_fds_to_close) {
      ::close(fd);
    }
    s_fds_to_close->clear();
  }
} s_os_extension;

} // anonymous namespace
} // namespace HPHP
