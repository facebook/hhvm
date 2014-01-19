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

#include "hphp/runtime/base/glob-stream-wrapper.h"

#include "hphp/runtime/base/directory.h"
#include "hphp/runtime/ext/ext_file.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

File* GlobStreamWrapper::open(const String& filename,
                              const String& mode,
                              int options,
                              CVarRef context) {
  // Can't open a glob as a file, it's meant to be opened as a directory

  // if the function was called via FCallBuiltin, we'll get a bogus name as
  // the stack frame will be wrong
  ActRec* ar = g_vmContext->getStackFrame();
  const char* fn = (ar != nullptr)
    ? ar->func()->name()->data()
    : "OPTIMIZED_BUILTIN";
  raise_warning("%s(%s): failed to open stream: "
                "wrapper does not support stream open",
                fn, filename.data());
  return nullptr;
}

Directory* GlobStreamWrapper::opendir(const String& path) {
  const char* prefix = "glob://";
  const char* path_str = path.data();
  int path_len = path.length();

  // only accept paths with the glob:// prefix
  if (strncmp(path_str, prefix, strlen(prefix)) != 0) {
    return nullptr;
  }

  path_str += strlen(prefix);
  path_len -= strlen(prefix);

  auto glob = f_glob(String(path_str, path_len, CopyString));
  if (!glob.isArray()) {
    return nullptr;
  }
  return NEWOBJ(ArrayDirectory)(glob.toArray());
}

///////////////////////////////////////////////////////////////////////////////
}
