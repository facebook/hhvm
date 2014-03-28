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
#include "hphp/runtime/base/stream-wrapper.h"

#include <sys/types.h>

#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/type-resource.h"
#include "hphp/runtime/base/directory.h"

namespace HPHP { namespace Stream {
///////////////////////////////////////////////////////////////////////////////

void Wrapper::registerAs(const std::string &scheme) {
  registerWrapper(scheme, this);
}

int access(const String& path, int mode)
{
  Stream::Wrapper *w = Stream::getWrapperFromURI(path);
  return !path.empty() && w ? w->access(path, mode) : -1;
}

int stat(const String& path, struct stat* buf)
{
  Stream::Wrapper *w = Stream::getWrapperFromURI(path);
  return !path.empty() && w ? w->stat(path, buf) : -1;
}

int lstat(const String& path, struct stat* buf)
{
  Stream::Wrapper* w = Stream::getWrapperFromURI(path);
  return !path.empty() && w ? w->lstat(path, buf) : -1;
}

Resource opendir(const String& path)
{
  Stream::Wrapper* w = Stream::getWrapperFromURI(path);
  Directory *directory = !path.empty() && w ? w->opendir(path) : nullptr;
  return Resource(directory);
}

int rmdir(const String& path, int options)
{
  Stream::Wrapper* w = Stream::getWrapperFromURI(path);
  return !path.empty() && w ? w->rmdir(path, options) : -1;
}

int mkdir(const String& path, int mode, int options)
{
  Stream::Wrapper* w = Stream::getWrapperFromURI(path);
  return !path.empty() && w ? w->mkdir(path, mode, options) : -1;
}

int unlink(const String& path)
{
  Stream::Wrapper* w = Stream::getWrapperFromURI(path);
  return !path.empty() && w ? w->unlink(path) : -1;
}

///////////////////////////////////////////////////////////////////////////////
}}
