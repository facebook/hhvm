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

/*
bool HPHP::f_dangling_server_proxy_old_request()
_ZN4HPHP35f_dangling_server_proxy_old_requestEv

(return value) => rax
*/

bool fh_dangling_server_proxy_old_request() asm("_ZN4HPHP35f_dangling_server_proxy_old_requestEv");

/*
bool HPHP::f_dangling_server_proxy_new_request(HPHP::String const&)
_ZN4HPHP35f_dangling_server_proxy_new_requestERKNS_6StringE

(return value) => rax
host => rdi
*/

bool fh_dangling_server_proxy_new_request(Value* host) asm("_ZN4HPHP35f_dangling_server_proxy_new_requestERKNS_6StringE");

/*
bool HPHP::f_pagelet_server_is_enabled()
_ZN4HPHP27f_pagelet_server_is_enabledEv

(return value) => rax
*/

bool fh_pagelet_server_is_enabled() asm("_ZN4HPHP27f_pagelet_server_is_enabledEv");

/*
HPHP::Object HPHP::f_pagelet_server_task_start(HPHP::String const&, HPHP::Array const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP27f_pagelet_server_task_startERKNS_6StringERKNS_5ArrayES2_S5_

(return value) => rax
_rv => rdi
url => rsi
headers => rdx
post_data => rcx
files => r8
*/

Value* fh_pagelet_server_task_start(Value* _rv, Value* url, Value* headers, Value* post_data, Value* files) asm("_ZN4HPHP27f_pagelet_server_task_startERKNS_6StringERKNS_5ArrayES2_S5_");

/*
long long HPHP::f_pagelet_server_task_status(HPHP::Object const&)
_ZN4HPHP28f_pagelet_server_task_statusERKNS_6ObjectE

(return value) => rax
task => rdi
*/

long long fh_pagelet_server_task_status(Value* task) asm("_ZN4HPHP28f_pagelet_server_task_statusERKNS_6ObjectE");

/*
HPHP::String HPHP::f_pagelet_server_task_result(HPHP::Object const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&, long long)
_ZN4HPHP28f_pagelet_server_task_resultERKNS_6ObjectERKNS_14VRefParamValueES5_x

(return value) => rax
_rv => rdi
task => rsi
headers => rdx
code => rcx
timeout_ms => r8
*/

Value* fh_pagelet_server_task_result(Value* _rv, Value* task, TypedValue* headers, TypedValue* code, long long timeout_ms) asm("_ZN4HPHP28f_pagelet_server_task_resultERKNS_6ObjectERKNS_14VRefParamValueES5_x");

/*
void HPHP::f_pagelet_server_flush()
_ZN4HPHP22f_pagelet_server_flushEv

*/

void fh_pagelet_server_flush() asm("_ZN4HPHP22f_pagelet_server_flushEv");

/*
bool HPHP::f_xbox_send_message(HPHP::String const&, HPHP::VRefParamValue const&, long long, HPHP::String const&)
_ZN4HPHP19f_xbox_send_messageERKNS_6StringERKNS_14VRefParamValueExS2_

(return value) => rax
msg => rdi
ret => rsi
timeout_ms => rdx
host => rcx
*/

bool fh_xbox_send_message(Value* msg, TypedValue* ret, long long timeout_ms, Value* host) asm("_ZN4HPHP19f_xbox_send_messageERKNS_6StringERKNS_14VRefParamValueExS2_");

/*
bool HPHP::f_xbox_post_message(HPHP::String const&, HPHP::String const&)
_ZN4HPHP19f_xbox_post_messageERKNS_6StringES2_

(return value) => rax
msg => rdi
host => rsi
*/

bool fh_xbox_post_message(Value* msg, Value* host) asm("_ZN4HPHP19f_xbox_post_messageERKNS_6StringES2_");

/*
HPHP::Object HPHP::f_xbox_task_start(HPHP::String const&)
_ZN4HPHP17f_xbox_task_startERKNS_6StringE

(return value) => rax
_rv => rdi
message => rsi
*/

Value* fh_xbox_task_start(Value* _rv, Value* message) asm("_ZN4HPHP17f_xbox_task_startERKNS_6StringE");

/*
bool HPHP::f_xbox_task_status(HPHP::Object const&)
_ZN4HPHP18f_xbox_task_statusERKNS_6ObjectE

(return value) => rax
task => rdi
*/

bool fh_xbox_task_status(Value* task) asm("_ZN4HPHP18f_xbox_task_statusERKNS_6ObjectE");

/*
long long HPHP::f_xbox_task_result(HPHP::Object const&, long long, HPHP::VRefParamValue const&)
_ZN4HPHP18f_xbox_task_resultERKNS_6ObjectExRKNS_14VRefParamValueE

(return value) => rax
task => rdi
timeout_ms => rsi
ret => rdx
*/

long long fh_xbox_task_result(Value* task, long long timeout_ms, TypedValue* ret) asm("_ZN4HPHP18f_xbox_task_resultERKNS_6ObjectExRKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::f_xbox_process_call_message(HPHP::String const&)
_ZN4HPHP27f_xbox_process_call_messageERKNS_6StringE

(return value) => rax
_rv => rdi
msg => rsi
*/

TypedValue* fh_xbox_process_call_message(TypedValue* _rv, Value* msg) asm("_ZN4HPHP27f_xbox_process_call_messageERKNS_6StringE");

/*
long long HPHP::f_xbox_get_thread_timeout()
_ZN4HPHP25f_xbox_get_thread_timeoutEv

(return value) => rax
*/

long long fh_xbox_get_thread_timeout() asm("_ZN4HPHP25f_xbox_get_thread_timeoutEv");

/*
void HPHP::f_xbox_set_thread_timeout(int)
_ZN4HPHP25f_xbox_set_thread_timeoutEi

timeout => rdi
*/

void fh_xbox_set_thread_timeout(int timeout) asm("_ZN4HPHP25f_xbox_set_thread_timeoutEi");

/*
void HPHP::f_xbox_schedule_thread_reset()
_ZN4HPHP28f_xbox_schedule_thread_resetEv

*/

void fh_xbox_schedule_thread_reset() asm("_ZN4HPHP28f_xbox_schedule_thread_resetEv");

/*
long long HPHP::f_xbox_get_thread_time()
_ZN4HPHP22f_xbox_get_thread_timeEv

(return value) => rax
*/

long long fh_xbox_get_thread_time() asm("_ZN4HPHP22f_xbox_get_thread_timeEv");


} // !HPHP

