/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#ifndef HPHP_STREAM_WRAPPER_REGISTRY_H
#define HPHP_STREAM_WRAPPER_REGISTRY_H

#include <map>
#include <string>
#include <memory>
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/stream_wrapper.h"

namespace HPHP { namespace Stream {
///////////////////////////////////////////////////////////////////////////////
class Wrapper;

bool registerWrapper(const std::string &scheme, Wrapper *wrapper);
bool disableWrapper(CStrRef scheme);
bool restoreWrapper(CStrRef scheme);
bool registerRequestWrapper(CStrRef scheme, std::unique_ptr<Wrapper> wrapper);
Array enumWrappers();
Wrapper* getWrapper(CStrRef scheme);
Wrapper* getWrapperFromURI(CStrRef uri);

/* Called during process init to register core wrappers */
void RegisterCoreWrappers();

///////////////////////////////////////////////////////////////////////////////
}}

#endif // HPHP_STREAM_WRAPPER_REGISTRY_H
