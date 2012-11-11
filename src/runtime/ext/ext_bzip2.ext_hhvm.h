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
HPHP::Variant HPHP::f_bzopen(HPHP::String const&, HPHP::String const&)
_ZN4HPHP8f_bzopenERKNS_6StringES2_

(return value) => rax
_rv => rdi
filename => rsi
mode => rdx
*/

TypedValue* fh_bzopen(TypedValue* _rv, Value* filename, Value* mode) asm("_ZN4HPHP8f_bzopenERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_bzflush(HPHP::Object const&)
_ZN4HPHP9f_bzflushERKNS_6ObjectE

(return value) => rax
_rv => rdi
bz => rsi
*/

TypedValue* fh_bzflush(TypedValue* _rv, Value* bz) asm("_ZN4HPHP9f_bzflushERKNS_6ObjectE");

/*
HPHP::String HPHP::f_bzerrstr(HPHP::Object const&)
_ZN4HPHP10f_bzerrstrERKNS_6ObjectE

(return value) => rax
_rv => rdi
bz => rsi
*/

Value* fh_bzerrstr(Value* _rv, Value* bz) asm("_ZN4HPHP10f_bzerrstrERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_bzerror(HPHP::Object const&)
_ZN4HPHP9f_bzerrorERKNS_6ObjectE

(return value) => rax
_rv => rdi
bz => rsi
*/

TypedValue* fh_bzerror(TypedValue* _rv, Value* bz) asm("_ZN4HPHP9f_bzerrorERKNS_6ObjectE");

/*
long long HPHP::f_bzerrno(HPHP::Object const&)
_ZN4HPHP9f_bzerrnoERKNS_6ObjectE

(return value) => rax
bz => rdi
*/

long long fh_bzerrno(Value* bz) asm("_ZN4HPHP9f_bzerrnoERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_bzcompress(HPHP::String const&, int, int)
_ZN4HPHP12f_bzcompressERKNS_6StringEii

(return value) => rax
_rv => rdi
source => rsi
blocksize => rdx
workfactor => rcx
*/

TypedValue* fh_bzcompress(TypedValue* _rv, Value* source, int blocksize, int workfactor) asm("_ZN4HPHP12f_bzcompressERKNS_6StringEii");

/*
HPHP::Variant HPHP::f_bzdecompress(HPHP::String const&, int)
_ZN4HPHP14f_bzdecompressERKNS_6StringEi

(return value) => rax
_rv => rdi
source => rsi
small => rdx
*/

TypedValue* fh_bzdecompress(TypedValue* _rv, Value* source, int small) asm("_ZN4HPHP14f_bzdecompressERKNS_6StringEi");


} // !HPHP

