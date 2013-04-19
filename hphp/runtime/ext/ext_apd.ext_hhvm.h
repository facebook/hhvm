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

bool fh_override_function(Value* name, Value* args, Value* code) asm("_ZN4HPHP19f_override_functionERKNS_6StringES2_S2_");

bool fh_rename_function(Value* orig_name, Value* new_name) asm("_ZN4HPHP17f_rename_functionERKNS_6StringES2_");

void fh_apd_set_browser_trace() asm("_ZN4HPHP23f_apd_set_browser_traceEv");

Value* fh_apd_set_pprof_trace(Value* _rv, Value* dumpdir, Value* frament) asm("_ZN4HPHP21f_apd_set_pprof_traceERKNS_6StringES2_");

bool fh_apd_set_session_trace_socket(Value* ip_or_filename, int domain, int port, int mask) asm("_ZN4HPHP30f_apd_set_session_trace_socketERKNS_6StringEiii");

void fh_apd_stop_trace() asm("_ZN4HPHP16f_apd_stop_traceEv");

bool fh_apd_breakpoint() asm("_ZN4HPHP16f_apd_breakpointEv");

bool fh_apd_continue() asm("_ZN4HPHP14f_apd_continueEv");

bool fh_apd_echo(Value* output) asm("_ZN4HPHP10f_apd_echoERKNS_6StringE");

} // namespace HPHP
