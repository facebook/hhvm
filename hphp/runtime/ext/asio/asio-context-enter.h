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

#ifndef incl_HPHP_EXT_ASIO_CONTEXT_ENTER_H_
#define incl_HPHP_EXT_ASIO_CONTEXT_ENTER_H_

#include "hphp/runtime/ext/asio/asio-context.h"
#include "hphp/runtime/ext/asio/waitable-wait-handle.h"

namespace HPHP { namespace asio {
///////////////////////////////////////////////////////////////////////////////

void enter_context(c_WaitableWaitHandle* root, context_idx_t ctx_idx);
void enter_context_impl(c_WaitableWaitHandle* root, context_idx_t ctx_idx);

///////////////////////////////////////////////////////////////////////////////
}}

#include "hphp/runtime/ext/asio/asio-context-enter-inl.h"

#endif // incl_HPHP_EXT_ASIO_CONTEXT_ENTER_H_
