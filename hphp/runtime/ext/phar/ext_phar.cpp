/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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
#include <sys/types.h>
#include <sys/stat.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/stream-wrapper.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/directory.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// phar:// stream wrapper

static const StaticString
  s_openPhar("openPhar"),
  s_stat("stat"),
  s_size("size"),
  s_mtime("mtime"),
  s_atime("atime"),
  s_ctime("ctime"),
  s_mode("mode"),
  s_opendir("opendir");

static struct PharStreamWrapper final : Stream::Wrapper {
  req::ptr<File>
  open(const String& filename, const String& /*mode*/, int /*options*/,
       const req::ptr<StreamContext>& /*context*/) override {
    static const char cz[] = "phar://";
    if (strncmp(filename.data(), cz, sizeof(cz) - 1)) {
      return nullptr;
    }

    static Func* f = SystemLib::s_PharClass->lookupMethod(s_openPhar.get());

    auto ret = Variant::attach(
      g_context->invokeFunc(f, make_packed_array(filename), nullptr,
                            SystemLib::s_PharClass)
    );

    if (!ret.isResource()) {
      return nullptr;
    }
    return dyn_cast_or_null<File>(ret.asResRef());
  }

  int access(const String& path, int /*mode*/) override {
    Variant ret = callStat(path);
    if (ret.isBoolean()) {
      assert(!ret.toBoolean());
      return -1;
    }
    return 0;
  }

  int stat(const String& path, struct stat* buf) override {
    Variant ret = callStat(path);
    if (ret.isBoolean()) {
      assert(!ret.toBoolean());
      return -1;
    }
    const Array& stat = ret.toArray();
    buf->st_size = stat[s_size].asInt64Val();
    buf->st_atime = stat[s_atime].asInt64Val();
    buf->st_mtime = stat[s_mtime].asInt64Val();
    buf->st_ctime = stat[s_ctime].asInt64Val();
    buf->st_mode = stat[s_mode].asInt64Val();
    return 0;
  }

  int lstat(const String& path, struct stat* buf) override {
    return stat(path, buf);
  }

  req::ptr<Directory> opendir(const String& path) override {
    static Func* f = SystemLib::s_PharClass->lookupMethod(s_opendir.get());
    auto ret = Variant::attach(
      g_context->invokeFunc(f, make_packed_array(path), nullptr,
                            SystemLib::s_PharClass)
    );
    Array files = ret.toArray();
    if (files.empty()) {
      raise_warning("No such file or directory");
      return nullptr;
    }
    return req::make<ArrayDirectory>(files);
  }

 private:
  Variant callStat(const String& path) {
    static Func* f = SystemLib::s_PharClass->lookupMethod(s_stat.get());
    return Variant::attach(
      g_context->invokeFunc(f, make_packed_array(path), nullptr,
                            SystemLib::s_PharClass)
    );
  }
} s_phar_stream_wrapper;

struct pharExtension final : Extension {
  pharExtension() : Extension("Phar", "2.0.2") {}
  void moduleInit() override {
    s_phar_stream_wrapper.registerAs("phar");
  }
} s_phar_extension;

///////////////////////////////////////////////////////////////////////////////
}
