/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_SERVER_H_
#define incl_HPHP_EXT_SERVER_H_

#include "hphp/runtime/ext/extension.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

enum PageletStatusType {
  PAGELET_NOT_READY,
  PAGELET_READY,
  PAGELET_DONE
};

///////////////////////////////////////////////////////////////////////////////

int64_t HHVM_FUNCTION(hphp_thread_type);
bool HHVM_FUNCTION(pagelet_server_is_enabled);
Resource HHVM_FUNCTION(pagelet_server_task_start,
                       const String& url,
                       const Array& headers = null_array,
                       const String& post_data = null_string,
                       const Array& files = null_array);
int64_t HHVM_FUNCTION(pagelet_server_task_status,
                      const Resource& task);
String HHVM_FUNCTION(pagelet_server_task_result,
                     const Resource& task,
                     VRefParam headers,
                     VRefParam code,
                     int64_t timeout_ms);
int64_t HHVM_FUNCTION(pagelet_server_tasks_started);
void HHVM_FUNCTION(pagelet_server_flush);
bool HHVM_FUNCTION(xbox_send_message,
                   const String& msg,
                   VRefParam ret,
                   int64_t timeout_ms,
                   const String& host = "localhost");
bool HHVM_FUNCTION(xbox_post_message,
                   const String& msg,
                   const String& host = "localhost");
Resource HHVM_FUNCTION(xbox_task_start,
                       const String& message);
bool HHVM_FUNCTION(xbox_task_status,
                   const Resource& task);
int64_t HHVM_FUNCTION(xbox_task_result,
                      const Resource& task,
                      int64_t timeout_ms,
                      VRefParam ret);
Variant HHVM_FUNCTION(xbox_process_call_message,
                      const String& msg);
int64_t HHVM_FUNCTION(xbox_get_thread_timeout);
void HHVM_FUNCTION(xbox_set_thread_timeout,
                   int timeout);
void HHVM_FUNCTION(xbox_schedule_thread_reset);
int64_t HHVM_FUNCTION(xbox_get_thread_time);
bool HHVM_FUNCTION(server_is_stopping);
bool HHVM_FUNCTION(server_is_prepared_to_stop);
int64_t HHVM_FUNCTION(server_health_level);
int64_t HHVM_FUNCTION(server_uptime);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_SERVER_H_
