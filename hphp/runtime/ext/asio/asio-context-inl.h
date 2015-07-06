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

#ifndef incl_HPHP_EXT_ASIO_CONTEXT_H_
#error "This should only be included by asio-context.h"
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

template <class TWaitHandle>
uint32_t AsioContext::registerTo(req::vector<TWaitHandle*>& vec,
                                 TWaitHandle* wh) {
  vec.push_back(wh);
  return vec.size() - 1;
}

template <class TWaitHandle>
void AsioContext::unregisterFrom(req::vector<TWaitHandle*>& vec,
                                 uint32_t idx) {
  assert(idx < vec.size());
  if (idx != vec.size() - 1) {
    vec[idx] = vec.back();
    vec[idx]->setContextVectorIndex(idx);
  }
  vec.pop_back();
}

///////////////////////////////////////////////////////////////////////////////
}
