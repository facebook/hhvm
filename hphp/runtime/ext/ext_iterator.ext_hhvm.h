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

Value* fh_hphp_recursiveiteratoriterator___construct(Value* _rv, Value* obj, Value* iterator, long mode, long flags) asm("_ZN4HPHP44f_hphp_recursiveiteratoriterator___constructERKNS_6ObjectES2_ll");

Value* fh_hphp_recursiveiteratoriterator_getinneriterator(Value* _rv, Value* obj) asm("_ZN4HPHP49f_hphp_recursiveiteratoriterator_getinneriteratorERKNS_6ObjectE");

TypedValue* fh_hphp_recursiveiteratoriterator_current(TypedValue* _rv, Value* obj) asm("_ZN4HPHP40f_hphp_recursiveiteratoriterator_currentERKNS_6ObjectE");

TypedValue* fh_hphp_recursivedirectoryiterator_current(TypedValue* _rv, Value* obj) asm("_ZN4HPHP41f_hphp_recursivedirectoryiterator_currentERKNS_6ObjectE");

TypedValue* fh_hphp_recursiveiteratoriterator_key(TypedValue* _rv, Value* obj) asm("_ZN4HPHP36f_hphp_recursiveiteratoriterator_keyERKNS_6ObjectE");

TypedValue* fh_hphp_recursivedirectoryiterator_key(TypedValue* _rv, Value* obj) asm("_ZN4HPHP37f_hphp_recursivedirectoryiterator_keyERKNS_6ObjectE");

void fh_hphp_recursiveiteratoriterator_next(Value* obj) asm("_ZN4HPHP37f_hphp_recursiveiteratoriterator_nextERKNS_6ObjectE");

bool fh_hphp_recursiveiteratoriterator_valid(Value* obj) asm("_ZN4HPHP38f_hphp_recursiveiteratoriterator_validERKNS_6ObjectE");

void fh_hphp_recursiveiteratoriterator_rewind(Value* obj) asm("_ZN4HPHP39f_hphp_recursiveiteratoriterator_rewindERKNS_6ObjectE");

void fh_hphp_recursivedirectoryiterator_rewind(Value* obj) asm("_ZN4HPHP40f_hphp_recursivedirectoryiterator_rewindERKNS_6ObjectE");

bool fh_hphp_directoryiterator___construct(Value* obj, Value* path) asm("_ZN4HPHP36f_hphp_directoryiterator___constructERKNS_6ObjectERKNS_6StringE");

TypedValue* fh_hphp_directoryiterator_key(TypedValue* _rv, Value* obj) asm("_ZN4HPHP28f_hphp_directoryiterator_keyERKNS_6ObjectE");

void fh_hphp_directoryiterator_next(Value* obj) asm("_ZN4HPHP29f_hphp_directoryiterator_nextERKNS_6ObjectE");

void fh_hphp_directoryiterator_rewind(Value* obj) asm("_ZN4HPHP31f_hphp_directoryiterator_rewindERKNS_6ObjectE");

void fh_hphp_directoryiterator_seek(Value* obj, long position) asm("_ZN4HPHP29f_hphp_directoryiterator_seekERKNS_6ObjectEl");

TypedValue* fh_hphp_directoryiterator_current(TypedValue* _rv, Value* obj) asm("_ZN4HPHP32f_hphp_directoryiterator_currentERKNS_6ObjectE");

Value* fh_hphp_directoryiterator___tostring(Value* _rv, Value* obj) asm("_ZN4HPHP35f_hphp_directoryiterator___tostringERKNS_6ObjectE");

bool fh_hphp_directoryiterator_valid(Value* obj) asm("_ZN4HPHP30f_hphp_directoryiterator_validERKNS_6ObjectE");

bool fh_hphp_directoryiterator_isdot(Value* obj) asm("_ZN4HPHP30f_hphp_directoryiterator_isdotERKNS_6ObjectE");

bool fh_hphp_recursivedirectoryiterator___construct(Value* obj, Value* path, long flags) asm("_ZN4HPHP45f_hphp_recursivedirectoryiterator___constructERKNS_6ObjectERKNS_6StringEl");

void fh_hphp_recursivedirectoryiterator_next(Value* obj) asm("_ZN4HPHP38f_hphp_recursivedirectoryiterator_nextERKNS_6ObjectE");

void fh_hphp_recursivedirectoryiterator_seek(Value* obj, long position) asm("_ZN4HPHP38f_hphp_recursivedirectoryiterator_seekERKNS_6ObjectEl");

Value* fh_hphp_recursivedirectoryiterator___tostring(Value* _rv, Value* obj) asm("_ZN4HPHP44f_hphp_recursivedirectoryiterator___tostringERKNS_6ObjectE");

bool fh_hphp_recursivedirectoryiterator_valid(Value* obj) asm("_ZN4HPHP39f_hphp_recursivedirectoryiterator_validERKNS_6ObjectE");

bool fh_hphp_recursivedirectoryiterator_haschildren(Value* obj) asm("_ZN4HPHP45f_hphp_recursivedirectoryiterator_haschildrenERKNS_6ObjectE");

Value* fh_hphp_recursivedirectoryiterator_getchildren(Value* _rv, Value* obj) asm("_ZN4HPHP45f_hphp_recursivedirectoryiterator_getchildrenERKNS_6ObjectE");

Value* fh_hphp_recursivedirectoryiterator_getsubpath(Value* _rv, Value* obj) asm("_ZN4HPHP44f_hphp_recursivedirectoryiterator_getsubpathERKNS_6ObjectE");

Value* fh_hphp_recursivedirectoryiterator_getsubpathname(Value* _rv, Value* obj) asm("_ZN4HPHP48f_hphp_recursivedirectoryiterator_getsubpathnameERKNS_6ObjectE");

} // namespace HPHP
