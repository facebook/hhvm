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

#include "hphp/runtime/ext/ext_phar.h"
#include "hphp/runtime/base/stream_wrapper.h"
#include "hphp/runtime/base/stream_wrapper_registry.h"
#include "hphp/runtime/base/mem_file.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// phar:// stream wrapper

static const StaticString
  s_openPhar("openPhar"),
  s_stat("stat"),
  s_size("size"),
  s_mtime("mtime"),
  s_atime("atime"),
  s_ctime("ctime");

static class PharStreamWrapper : public Stream::Wrapper {
 public:
  virtual File* open(CStrRef filename, CStrRef mode,
                     int options, CVarRef context) {
    static const char cz[] = "phar://";
    if (strncmp(filename.data(), cz, sizeof(cz) - 1)) {
      return nullptr;
    }

    static Func* f = SystemLib::s_PharClass->lookupMethod(s_openPhar.get());

    Variant ret;
    g_vmContext->invokeFunc(
      ret.asTypedValue(),
      f,
      CREATE_VECTOR1(filename),
      nullptr,
      SystemLib::s_PharClass
    );
    CStrRef contents = ret.toString();

    MemFile* file = NEWOBJ(MemFile)(contents.data(), contents.size());
    return file;
  }

  virtual int access(CStrRef path, int mode) {
    Variant ret = callStat(path);
    if (ret.isBoolean()) {
      assert(!ret.toBoolean());
      return -1;
    }
    return 0;
  }

  virtual int stat(CStrRef path, struct stat* buf) {
    Variant ret = callStat(path);
    if (ret.isBoolean()) {
      assert(!ret.toBoolean());
      return -1;
    }
    CArrRef stat = ret.toArray();
    buf->st_size = stat[s_size].asInt64Val();
    buf->st_atime = stat[s_atime].asInt64Val();
    buf->st_mtime = stat[s_mtime].asInt64Val();
    buf->st_ctime = stat[s_ctime].asInt64Val();
    return 0;
  }

  virtual int lstat(CStrRef path, struct stat* buf) {
    return stat(path, buf);
  }

 private:
  Variant callStat(CStrRef path) {
    static Func* f = SystemLib::s_PharClass->lookupMethod(s_stat.get());
    Variant ret;
    g_vmContext->invokeFunc(
      ret.asTypedValue(),
      f,
      CREATE_VECTOR1(path),
      nullptr,
      SystemLib::s_PharClass
    );
    return ret;
  }

} s_phar_stream_wrapper;

void pharExtension::moduleInit() {
  s_phar_stream_wrapper.registerAs("phar");
}
pharExtension s_phar_extension;

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
}
