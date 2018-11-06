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
#ifndef incl_HPHP_EXECUTION_PROFILER_H_
#define incl_HPHP_EXECUTION_PROFILER_H_

#include "hphp/runtime/base/request-info.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct ExecutionProfiler {
  ExecutionProfiler(RequestInfo *info, bool builtin) : m_info(info) {
    m_executing = m_info->m_executing;
    m_info->m_executing =
      builtin ? RequestInfo::ExtensionFunctions : RequestInfo::UserFunctions;
  }
  explicit ExecutionProfiler(RequestInfo::Executing executing) {
    m_info = RequestInfo::s_requestInfo.getNoCheck();
    m_executing = m_info->m_executing;
    m_info->m_executing = executing;
  }
  ~ExecutionProfiler() {
    m_info->m_executing = m_executing;
  }
private:
  RequestInfo *m_info;
  RequestInfo::Executing m_executing;
};

//////////////////////////////////////////////////////////////////////

}

#endif
