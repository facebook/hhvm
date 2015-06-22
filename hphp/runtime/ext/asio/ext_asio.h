/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_ASIO_H_
#define incl_HPHP_EXT_ASIO_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/asio/async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-generator.h"
#include "hphp/runtime/ext/asio/async-generator-wait-handle.h"
#include "hphp/runtime/ext/asio/await-all-wait-handle.h"
#include "hphp/runtime/ext/asio/condition-wait-handle.h"
#include "hphp/runtime/ext/asio/external-thread-event-wait-handle.h"
#include "hphp/runtime/ext/asio/gen-array-wait-handle.h"
#include "hphp/runtime/ext/asio/gen-map-wait-handle.h"
#include "hphp/runtime/ext/asio/gen-vector-wait-handle.h"
#include "hphp/runtime/ext/asio/reschedule-wait-handle.h"
#include "hphp/runtime/ext/asio/sleep-wait-handle.h"
#include "hphp/runtime/ext/asio/static-wait-handle.h"
#include "hphp/runtime/ext/asio/waitable-wait-handle.h"

namespace HPHP {

Object HHVM_FUNCTION(asio_get_running);

}

#endif // incl_HPHP_EXT_ASIO_H_
