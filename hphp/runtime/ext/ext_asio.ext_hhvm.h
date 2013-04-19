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

void fh_asio_enter_context() asm("_ZN4HPHP20f_asio_enter_contextEv");

void fh_asio_exit_context() asm("_ZN4HPHP19f_asio_exit_contextEv");

int fh_asio_get_current_context_idx() asm("_ZN4HPHP30f_asio_get_current_context_idxEv");

Value* fh_asio_get_running_in_context(Value* _rv, int ctx_idx) asm("_ZN4HPHP29f_asio_get_running_in_contextEi");

Value* fh_asio_get_running(Value* _rv) asm("_ZN4HPHP18f_asio_get_runningEv");

Value* fh_asio_get_current(Value* _rv) asm("_ZN4HPHP18f_asio_get_currentEv");

void fh_asio_set_on_failed_callback(TypedValue* on_failed_cb) asm("_ZN4HPHP29f_asio_set_on_failed_callbackERKNS_7VariantE");

void fh_asio_set_on_started_callback(TypedValue* on_started_cb) asm("_ZN4HPHP30f_asio_set_on_started_callbackERKNS_7VariantE");

} // namespace HPHP
