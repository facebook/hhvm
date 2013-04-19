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

TypedValue* fh_bzclose(TypedValue* _rv, Value* bz) asm("_ZN4HPHP9f_bzcloseERKNS_6ObjectE");

TypedValue* fh_bzread(TypedValue* _rv, Value* bz, int length) asm("_ZN4HPHP8f_bzreadERKNS_6ObjectEi");

TypedValue* fh_bzwrite(TypedValue* _rv, Value* bz, Value* data, int length) asm("_ZN4HPHP9f_bzwriteERKNS_6ObjectERKNS_6StringEi");

TypedValue* fh_bzopen(TypedValue* _rv, TypedValue* filename, Value* mode) asm("_ZN4HPHP8f_bzopenERKNS_7VariantERKNS_6StringE");

TypedValue* fh_bzflush(TypedValue* _rv, Value* bz) asm("_ZN4HPHP9f_bzflushERKNS_6ObjectE");

Value* fh_bzerrstr(Value* _rv, Value* bz) asm("_ZN4HPHP10f_bzerrstrERKNS_6ObjectE");

TypedValue* fh_bzerror(TypedValue* _rv, Value* bz) asm("_ZN4HPHP9f_bzerrorERKNS_6ObjectE");

long fh_bzerrno(Value* bz) asm("_ZN4HPHP9f_bzerrnoERKNS_6ObjectE");

TypedValue* fh_bzcompress(TypedValue* _rv, Value* source, int blocksize, int workfactor) asm("_ZN4HPHP12f_bzcompressERKNS_6StringEii");

TypedValue* fh_bzdecompress(TypedValue* _rv, Value* source, int small) asm("_ZN4HPHP14f_bzdecompressERKNS_6StringEi");

} // namespace HPHP
