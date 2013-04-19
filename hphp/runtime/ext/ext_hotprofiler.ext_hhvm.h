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

void fh_fb_setprofile(TypedValue* callback) asm("_ZN4HPHP15f_fb_setprofileERKNS_7VariantE");

void fh_xhprof_frame_begin(Value* name) asm("_ZN4HPHP20f_xhprof_frame_beginERKNS_6StringE");

void fh_xhprof_frame_end() asm("_ZN4HPHP18f_xhprof_frame_endEv");

void fh_xhprof_enable(int flags, Value* args) asm("_ZN4HPHP15f_xhprof_enableEiRKNS_5ArrayE");

TypedValue* fh_xhprof_disable(TypedValue* _rv) asm("_ZN4HPHP16f_xhprof_disableEv");

void fh_xhprof_network_enable() asm("_ZN4HPHP23f_xhprof_network_enableEv");

TypedValue* fh_xhprof_network_disable(TypedValue* _rv) asm("_ZN4HPHP24f_xhprof_network_disableEv");

void fh_xhprof_sample_enable() asm("_ZN4HPHP22f_xhprof_sample_enableEv");

TypedValue* fh_xhprof_sample_disable(TypedValue* _rv) asm("_ZN4HPHP23f_xhprof_sample_disableEv");

TypedValue* fh_xhprof_run_trace(TypedValue* _rv, Value* packedTrace, int flags) asm("_ZN4HPHP18f_xhprof_run_traceERKNS_6StringEi");

} // namespace HPHP
