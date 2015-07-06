/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/server/http-protocol.h"
#include "hphp/runtime/ext/stream/ext_stream.h"
#include "hphp/runtime/ext/stream/ext_stream-user-filters.h"
#include <memory>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const StaticString s_php("PHP");
const StaticString s_input("Input");
const StaticString s_temp("TEMP");
const StaticString s_memory("MEMORY");

req::ptr<File> PhpStreamWrapper::openFD(const char *sFD) {
  if (!RuntimeOption::ClientExecutionMode()) {
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

static void phpStreamApplyFilterList(const Resource& fpres,
                                     char *filter,
                                     int rwMode) {
  char *token = nullptr;
  filter = strtok_r(filter, "|", &token);
  while (filter) {
    auto ret = HHVM_FN(stream_filter_append)(fpres, String(filter, CopyString),
                                             rwMode, null_variant);
    if (!ret.toBoolean()) {
      raise_warning("Unable to create filter (%s)", filter);
    }
    filter = strtok_r(nullptr, "|", &token);
  }
}

static req::ptr<File>
phpStreamOpenFilter(const char* sFilter,
                    const String& modestr,
                    int options,
                    const req::ptr<StreamContext>& context) {
  const char *mode = modestr.c_str();
  int rwMode = 0;
  if (strchr(mode, 'r') || strchr(mode, '+')) {
    rwMode |= k_STREAM_FILTER_READ;
  }
  if (strchr(mode, 'w') || strchr(mode, '+') || strchr(mode, 'a')) {
    rwMode |= k_STREAM_FILTER_WRITE;
  }

  String duppath(sFilter, CopyString);
  char *path = duppath.mutableData();
  char *p = strstr(path, "/resource=");
  if (!p) {
    raise_recoverable_error("No URL resource specified");
    return nullptr;
  }
  auto fp = File::Open(String(p + sizeof("/resource=") - 1, CopyString),
                       modestr, options, context);
  if (!fp) return nullptr;
  Resource fpres(fp);
  *p = 0;
  char *token = nullptr;
  p = strtok_r(path + 1, "/", &token);
  while (p) {
    if (!strncasecmp(p, "read=", sizeof("read=") - 1)) {
      phpStreamApplyFilterList(fpres, p + sizeof("read=") - 1,
                               k_STREAM_FILTER_READ);
    } else if (!strncasecmp(p, "write=", sizeof("write=") - 1)) {
      phpStreamApplyFilterList(fpres, p + sizeof("write=") - 1,
                               k_STREAM_FILTER_WRITE);
    } else {
      phpStreamApplyFilterList(fpres, p, rwMode);
    }
    p = strtok_r(nullptr, "/", &token);
  }
  return fp;
}

req::ptr<File>
PhpStreamWrapper::open(const String& filename, const String& mode,
                       int options, const req::ptr<StreamContext>& context) {
  if (strncasecmp(filename.c_str(), "php://", 6)) {
    return nullptr;
  }

  const char *req = filename.c_str() + sizeof("php://") - 1;

  if (!strcasecmp(req, "stdin")) {
    return req::make<PlainFile>(dup(STDIN_FILENO), true, s_php);
  }
  if (!strcasecmp(req, "stdout")) {
    return req::make<PlainFile>(dup(STDOUT_FILENO), true, s_php);
  }
  if (!strcasecmp(req, "stderr")) {
    return req::make<PlainFile>(dup(STDERR_FILENO), true, s_php);
  }
  if (!strncasecmp(req, "fd/", sizeof("fd/") - 1)) {
    return openFD(req + sizeof("fd/") - 1);
  }
  if (!strncasecmp(req, "filter/", sizeof("filter/") - 1)) {
    return phpStreamOpenFilter(req + sizeof("filter") - 1, mode,
                               options, context);
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
