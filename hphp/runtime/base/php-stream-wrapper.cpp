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

#include "hphp/runtime/base/php-stream-wrapper.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/base/temp-file.h"
#include "hphp/runtime/base/mem-file.h"
#include "hphp/runtime/base/output-file.h"
#include "hphp/runtime/server/cli-server.h"
#include "hphp/runtime/server/http-protocol.h"
#include "hphp/runtime/ext/stream/ext_stream.h"
#include <memory>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const StaticString s_php("PHP");
const StaticString s_input("Input");
const StaticString s_temp("TEMP");
const StaticString s_memory("MEMORY");

req::ptr<File> PhpStreamWrapper::openFD(const char *sFD) {
  if (is_cli_server_mode()) {
    raise_warning("Direct access to file descriptors is not "
                  "available via remote unix server execution");
    return nullptr;
  }
  if (RuntimeOption::ServerExecutionMode()) {
    raise_warning("Direct access to file descriptors "
                  "is only available from command-line");
    return nullptr;
  }

  char *end = nullptr;
  long nFD = strtol(sFD, &end, 10);
  if ((sFD == end) || (*end != '\0')) {
    raise_warning("php://fd/ stream must be specified in the form "
                  "php://fd/<orig fd>");
    return nullptr;
  }
  long dtablesize = getdtablesize();
  if ((nFD < 0) || (nFD >= dtablesize)) {
    raise_warning("The file descriptors must be non-negative numbers "
                  "smaller than %ld", dtablesize);
    return nullptr;
  }

  return req::make<PlainFile>(dup(nFD), true, s_php);
}

req::ptr<File>
PhpStreamWrapper::open(const String& filename, const String& mode,
                       int options, const req::ptr<StreamContext>& context) {
  if (strncasecmp(filename.c_str(), "php://", 6)) {
    return nullptr;
  }

  const char *req = filename.c_str() + sizeof("php://") - 1;

  auto make_from = [] (const Variant& f) -> req::ptr<File> {
    auto res = dyn_cast_or_null<PlainFile>(f);
    if (!res || res->isClosed()) return nullptr;
    return req::make<PlainFile>(dup(res->fd()), true, s_php);
  };

  if (!strcasecmp(req, "stdin")) {
    return make_from(BuiltinFiles::getSTDIN());
  }
  if (!strcasecmp(req, "stdout")) {
    return make_from(BuiltinFiles::getSTDOUT());
  }
  if (!strcasecmp(req, "stderr")) {
    return make_from(BuiltinFiles::getSTDERR());
  }
  if (!strncasecmp(req, "fd/", sizeof("fd/") - 1)) {
    return openFD(req + sizeof("fd/") - 1);
  }
  if (!strncasecmp(req, "temp", sizeof("temp") - 1)) {
    auto file = req::make<TempFile>(true, s_php, s_temp);
    if (!file->valid()) {
      raise_warning("Unable to create temporary file");
      return nullptr;
    }
    return file;
  }
  if (!strcasecmp(req, "memory")) {
    auto file = req::make<TempFile>(true, s_php, s_memory);
    if (!file->valid()) {
      raise_warning("Unable to create temporary file");
      return nullptr;
    }

    file->getData()->m_mode = [mode] {
      if (mode.empty()) {
        return "w+b";
      }
      for (auto c : mode.slice()) {
        switch (c) {
          case '+':
          case 'w':
          case 'a':
          case 'x':
          case 'c':
            return "w+b";
          default:
            break;
        }
      }
      return "rb";
    }();
    return file;
  }

  if (!strcasecmp(req, "input")) {
    auto raw_post = g_context->getRawPostData();
    return req::make<MemFile>(
      raw_post.c_str(), raw_post.size(), s_php, s_input);
  }

  if (!strcasecmp(req, "output")) {
    return req::make<OutputFile>(filename);
  }

  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
}
