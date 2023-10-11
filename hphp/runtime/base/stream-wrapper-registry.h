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

#ifndef HPHP_STREAM_WRAPPER_REGISTRY_H
#define HPHP_STREAM_WRAPPER_REGISTRY_H

#include <map>
#include <string>
#include <memory>
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/stream-wrapper.h"

namespace HPHP::Stream {
///////////////////////////////////////////////////////////////////////////////
struct Wrapper;

bool registerWrapper(const std::string &scheme, Wrapper *wrapper);
Array enumWrappers();

String getWrapperProtocol(const char* url, int* pathIndex = nullptr);
Wrapper* getWrapper(const String& scheme, bool warn = true);
Wrapper* getWrapperFromURI(const String& uri,
                           int* pathIndex = nullptr, bool warn = true);

/* Called during process init to register core wrappers */
void RegisterCoreWrappers();

Wrapper* getThreadLocalFileHandler();
void setThreadLocalFileHandler(Wrapper* wrapper);

///////////////////////////////////////////////////////////////////////////////
}

#endif // HPHP_STREAM_WRAPPER_REGISTRY_H
