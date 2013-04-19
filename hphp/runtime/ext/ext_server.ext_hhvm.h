/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
namespace HPHP {

bool fh_dangling_server_proxy_old_request() asm("_ZN4HPHP35f_dangling_server_proxy_old_requestEv");

bool fh_dangling_server_proxy_new_request(Value* host) asm("_ZN4HPHP35f_dangling_server_proxy_new_requestERKNS_6StringE");

bool fh_pagelet_server_is_enabled() asm("_ZN4HPHP27f_pagelet_server_is_enabledEv");

Value* fh_pagelet_server_task_start(Value* _rv, Value* url, Value* headers, Value* post_data, Value* files) asm("_ZN4HPHP27f_pagelet_server_task_startERKNS_6StringERKNS_5ArrayES2_S5_");

long fh_pagelet_server_task_status(Value* task) asm("_ZN4HPHP28f_pagelet_server_task_statusERKNS_6ObjectE");

Value* fh_pagelet_server_task_result(Value* _rv, Value* task, TypedValue* headers, TypedValue* code, long timeout_ms) asm("_ZN4HPHP28f_pagelet_server_task_resultERKNS_6ObjectERKNS_14VRefParamValueES5_l");

void fh_pagelet_server_flush() asm("_ZN4HPHP22f_pagelet_server_flushEv");

bool fh_xbox_send_message(Value* msg, TypedValue* ret, long timeout_ms, Value* host) asm("_ZN4HPHP19f_xbox_send_messageERKNS_6StringERKNS_14VRefParamValueElS2_");

bool fh_xbox_post_message(Value* msg, Value* host) asm("_ZN4HPHP19f_xbox_post_messageERKNS_6StringES2_");

Value* fh_xbox_task_start(Value* _rv, Value* message) asm("_ZN4HPHP17f_xbox_task_startERKNS_6StringE");

bool fh_xbox_task_status(Value* task) asm("_ZN4HPHP18f_xbox_task_statusERKNS_6ObjectE");

long fh_xbox_task_result(Value* task, long timeout_ms, TypedValue* ret) asm("_ZN4HPHP18f_xbox_task_resultERKNS_6ObjectElRKNS_14VRefParamValueE");

TypedValue* fh_xbox_process_call_message(TypedValue* _rv, Value* msg) asm("_ZN4HPHP27f_xbox_process_call_messageERKNS_6StringE");

long fh_xbox_get_thread_timeout() asm("_ZN4HPHP25f_xbox_get_thread_timeoutEv");

void fh_xbox_set_thread_timeout(int timeout) asm("_ZN4HPHP25f_xbox_set_thread_timeoutEi");

void fh_xbox_schedule_thread_reset() asm("_ZN4HPHP28f_xbox_schedule_thread_resetEv");

long fh_xbox_get_thread_time() asm("_ZN4HPHP22f_xbox_get_thread_timeEv");

} // namespace HPHP
