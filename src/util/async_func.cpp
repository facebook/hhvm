/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "async_func.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

typedef void PFN_THREAD_FUNC(void *);

PFN_THREAD_FUNC* AsyncFuncImpl::s_initFunc = NULL;
void* AsyncFuncImpl::s_initFuncArg = NULL;

PFN_THREAD_FUNC* AsyncFuncImpl::s_finiFunc = NULL;
void* AsyncFuncImpl::s_finiFuncArg = NULL;

///////////////////////////////////////////////////////////////////////////////
}
