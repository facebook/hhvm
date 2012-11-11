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
bool HPHP::f_hphp_is_service_thread()
_ZN4HPHP24f_hphp_is_service_threadEv

(return value) => rax
*/

bool fh_hphp_is_service_thread() asm("_ZN4HPHP24f_hphp_is_service_threadEv");

/*
void HPHP::f_hphp_service_thread_started()
_ZN4HPHP29f_hphp_service_thread_startedEv

*/

void fh_hphp_service_thread_started() asm("_ZN4HPHP29f_hphp_service_thread_startedEv");

/*
bool HPHP::f_hphp_service_thread_stopped(int)
_ZN4HPHP29f_hphp_service_thread_stoppedEi

(return value) => rax
timeout => rdi
*/

bool fh_hphp_service_thread_stopped(int timeout) asm("_ZN4HPHP29f_hphp_service_thread_stoppedEi");

/*
bool HPHP::f_hphp_thread_is_warmup_enabled()
_ZN4HPHP31f_hphp_thread_is_warmup_enabledEv

(return value) => rax
*/

bool fh_hphp_thread_is_warmup_enabled() asm("_ZN4HPHP31f_hphp_thread_is_warmup_enabledEv");

/*
void HPHP::f_hphp_thread_set_warmup_enabled()
_ZN4HPHP32f_hphp_thread_set_warmup_enabledEv

*/

void fh_hphp_thread_set_warmup_enabled() asm("_ZN4HPHP32f_hphp_thread_set_warmup_enabledEv");

/*
long long HPHP::f_hphp_get_thread_id()
_ZN4HPHP20f_hphp_get_thread_idEv

(return value) => rax
*/

long long fh_hphp_get_thread_id() asm("_ZN4HPHP20f_hphp_get_thread_idEv");

/*
int HPHP::f_hphp_gettid()
_ZN4HPHP13f_hphp_gettidEv

(return value) => rax
*/

int fh_hphp_gettid() asm("_ZN4HPHP13f_hphp_gettidEv");


} // !HPHP

