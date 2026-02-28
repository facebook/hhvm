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

#pragma once

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/stream/ext_stream.h"

// To get the values of the SEEK constants
#include <stdio.h>

#undef basename

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// constants

#define PHP_FILE_USE_INCLUDE_PATH   1
#define PHP_FILE_IGNORE_NEW_LINES   2
#define PHP_FILE_SKIP_EMPTY_LINES   4
#define PHP_FILE_APPEND             8
#define PHP_FILE_NO_DEFAULT_CONTEXT 16

#ifndef GLOB_ONLYDIR
# define GLOB_ONLYDIR (1<<30)
# define GLOB_EMULATE_ONLYDIR
# define GLOB_FLAGMASK (~GLOB_ONLYDIR)
#else
# define GLOB_FLAGMASK (~0)
#endif

#define PHP_GLOB_FLAGS (0 | GLOB_BRACE | GLOB_MARK  \
                          | GLOB_NOSORT | GLOB_NOCHECK \
                          | GLOB_NOESCAPE | GLOB_ERR \
                          | GLOB_ONLYDIR)
#define PHP_GLOB_FLAGMASK (GLOB_FLAGMASK & PHP_GLOB_FLAGS)

#ifdef _MSC_VER
const StaticString s_DIRECTORY_SEPARATOR("\\");
const StaticString s_PATH_SEPARATOR(";");
#else
const StaticString s_DIRECTORY_SEPARATOR("/");
const StaticString s_PATH_SEPARATOR(":");
#endif
const int64_t k_LOCK_SH = 1;
const int64_t k_LOCK_EX = 2;
const int64_t k_LOCK_UN = 3;
const int64_t k_LOCK_NB = 4;
const int64_t k_SCANDIR_SORT_ASCENDING = 0;
const int64_t k_SCANDIR_SORT_DESCENDING = 1;
const int64_t k_SCANDIR_SORT_NONE = 2;

constexpr int64_t k_INI_SCANNER_NORMAL = 0;
constexpr int64_t k_INI_SCANNER_RAW = 1;

///////////////////////////////////////////////////////////////////////////////
// file handle based file operations

Variant HHVM_FUNCTION(fopen,
                      const String& filename,
                      const String& mode,
                      bool use_include_path = false,
                      const Variant& context = uninit_null());
bool HHVM_FUNCTION(fclose, const OptResource& handle);
Variant HHVM_FUNCTION(fseek,
                      const OptResource& handle,
                      int64_t offset,
                      int64_t whence = SEEK_SET);
bool HHVM_FUNCTION(rewind, const OptResource& handle);
Variant HHVM_FUNCTION(ftell, const OptResource& handle);
bool HHVM_FUNCTION(feof, const OptResource& handle);
Variant HHVM_FUNCTION(fstat, const OptResource& handle);
Variant HHVM_FUNCTION(fread,
                      const OptResource& handle,
                      int64_t length);
Variant HHVM_FUNCTION(fgetc, const OptResource& handle);
Variant HHVM_FUNCTION(fgets,
                      const OptResource& handle,
                      int64_t length = 0);
Variant HHVM_FUNCTION(fgetss,
                      const OptResource& handle,
                      int64_t length = 0,
                      const String& allowable_tags = null_string);
Variant HHVM_FUNCTION(fpassthru, const OptResource& handle);
Variant HHVM_FUNCTION(fwrite,
                      const OptResource& handle,
                      const String& data,
                      int64_t length = 0);

///////////////////////////////////////////////////////////////////////////////
// file name based file operations

Variant HHVM_FUNCTION(file_get_contents,
                      const String& filename,
                      bool use_include_path = false,
                      const Variant& context = uninit_null(),
                      int64_t offset = -1,
                      int64_t maxlen = -1);
Variant HHVM_FUNCTION(readfile,
                      const String& filename,
                      bool use_include_path = false,
                      const Variant& context = uninit_null());

///////////////////////////////////////////////////////////////////////////////
// shell commands

String HHVM_FUNCTION(basename,
                     const String& path,
                     const String& suffix = null_string);
Variant HHVM_FUNCTION(glob,
                      const String& pattern,
                      int64_t flags = 0);

///////////////////////////////////////////////////////////////////////////////
// stats functions

bool HHVM_FUNCTION(is_writable,
                   const String& filename);
bool HHVM_FUNCTION(is_readable,
                   const String& filename);
bool HHVM_FUNCTION(is_file,
                   const String& filename);
bool HHVM_FUNCTION(is_dir,
                   const String& filename);
bool HHVM_FUNCTION(file_exists,
                   const String& filename);
Variant HHVM_FUNCTION(realpath,
                      const String& path);
Variant HHVM_FUNCTION(pathinfo,
                      const String& path,
                      int64_t opt = 15);

///////////////////////////////////////////////////////////////////////////////
// directory functions

bool HHVM_FUNCTION(mkdir,
                   const String& pathname,
                   int64_t mode = 0777,
                   bool recursive = false,
                   const Variant& context = uninit_null());
String HHVM_FUNCTION(dirname,
                     const String& path);
Variant HHVM_FUNCTION(getcwd);
Variant HHVM_FUNCTION(readdir,
                      const Variant& dir_handle = uninit_variant);
Variant HHVM_FUNCTION(scandir,
                      const String& directory,
                      bool descending = false,
                      const Variant& context = uninit_null());

///////////////////////////////////////////////////////////////////////////////
}
