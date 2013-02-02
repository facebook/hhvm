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
HPHP::Variant HPHP::f_readgzfile(HPHP::String const&, bool)
_ZN4HPHP12f_readgzfileERKNS_6StringEb

(return value) => rax
_rv => rdi
filename => rsi
use_include_path => rdx
*/

TypedValue* fh_readgzfile(TypedValue* _rv, Value* filename, bool use_include_path) asm("_ZN4HPHP12f_readgzfileERKNS_6StringEb");

/*
HPHP::Variant HPHP::f_gzfile(HPHP::String const&, bool)
_ZN4HPHP8f_gzfileERKNS_6StringEb

(return value) => rax
_rv => rdi
filename => rsi
use_include_path => rdx
*/

TypedValue* fh_gzfile(TypedValue* _rv, Value* filename, bool use_include_path) asm("_ZN4HPHP8f_gzfileERKNS_6StringEb");

/*
HPHP::Variant HPHP::f_gzcompress(HPHP::String const&, int)
_ZN4HPHP12f_gzcompressERKNS_6StringEi

(return value) => rax
_rv => rdi
data => rsi
level => rdx
*/

TypedValue* fh_gzcompress(TypedValue* _rv, Value* data, int level) asm("_ZN4HPHP12f_gzcompressERKNS_6StringEi");

/*
HPHP::Variant HPHP::f_gzuncompress(HPHP::String const&, int)
_ZN4HPHP14f_gzuncompressERKNS_6StringEi

(return value) => rax
_rv => rdi
data => rsi
limit => rdx
*/

TypedValue* fh_gzuncompress(TypedValue* _rv, Value* data, int limit) asm("_ZN4HPHP14f_gzuncompressERKNS_6StringEi");

/*
HPHP::Variant HPHP::f_gzdeflate(HPHP::String const&, int)
_ZN4HPHP11f_gzdeflateERKNS_6StringEi

(return value) => rax
_rv => rdi
data => rsi
level => rdx
*/

TypedValue* fh_gzdeflate(TypedValue* _rv, Value* data, int level) asm("_ZN4HPHP11f_gzdeflateERKNS_6StringEi");

/*
HPHP::Variant HPHP::f_gzinflate(HPHP::String const&, int)
_ZN4HPHP11f_gzinflateERKNS_6StringEi

(return value) => rax
_rv => rdi
data => rsi
limit => rdx
*/

TypedValue* fh_gzinflate(TypedValue* _rv, Value* data, int limit) asm("_ZN4HPHP11f_gzinflateERKNS_6StringEi");

/*
HPHP::Variant HPHP::f_gzencode(HPHP::String const&, int, int)
_ZN4HPHP10f_gzencodeERKNS_6StringEii

(return value) => rax
_rv => rdi
data => rsi
level => rdx
encoding_mode => rcx
*/

TypedValue* fh_gzencode(TypedValue* _rv, Value* data, int level, int encoding_mode) asm("_ZN4HPHP10f_gzencodeERKNS_6StringEii");

/*
HPHP::Variant HPHP::f_gzdecode(HPHP::String const&)
_ZN4HPHP10f_gzdecodeERKNS_6StringE

(return value) => rax
_rv => rdi
data => rsi
*/

TypedValue* fh_gzdecode(TypedValue* _rv, Value* data) asm("_ZN4HPHP10f_gzdecodeERKNS_6StringE");

/*
HPHP::Object HPHP::f_gzopen(HPHP::String const&, HPHP::String const&, bool)
_ZN4HPHP8f_gzopenERKNS_6StringES2_b

(return value) => rax
_rv => rdi
filename => rsi
mode => rdx
use_include_path => rcx
*/

Value* fh_gzopen(Value* _rv, Value* filename, Value* mode, bool use_include_path) asm("_ZN4HPHP8f_gzopenERKNS_6StringES2_b");

/*
HPHP::Variant HPHP::f_qlzcompress(HPHP::String const&, int)
_ZN4HPHP13f_qlzcompressERKNS_6StringEi

(return value) => rax
_rv => rdi
data => rsi
level => rdx
*/

TypedValue* fh_qlzcompress(TypedValue* _rv, Value* data, int level) asm("_ZN4HPHP13f_qlzcompressERKNS_6StringEi");

/*
HPHP::Variant HPHP::f_qlzuncompress(HPHP::String const&, int)
_ZN4HPHP15f_qlzuncompressERKNS_6StringEi

(return value) => rax
_rv => rdi
data => rsi
level => rdx
*/

TypedValue* fh_qlzuncompress(TypedValue* _rv, Value* data, int level) asm("_ZN4HPHP15f_qlzuncompressERKNS_6StringEi");

/*
HPHP::Variant HPHP::f_sncompress(HPHP::String const&)
_ZN4HPHP12f_sncompressERKNS_6StringE

(return value) => rax
_rv => rdi
data => rsi
*/

TypedValue* fh_sncompress(TypedValue* _rv, Value* data) asm("_ZN4HPHP12f_sncompressERKNS_6StringE");

/*
HPHP::Variant HPHP::f_snuncompress(HPHP::String const&)
_ZN4HPHP14f_snuncompressERKNS_6StringE

(return value) => rax
_rv => rdi
data => rsi
*/

TypedValue* fh_snuncompress(TypedValue* _rv, Value* data) asm("_ZN4HPHP14f_snuncompressERKNS_6StringE");

/*
HPHP::Variant HPHP::f_nzcompress(HPHP::String const&)
_ZN4HPHP12f_nzcompressERKNS_6StringE

(return value) => rax
_rv => rdi
uncompressed => rsi
*/

TypedValue* fh_nzcompress(TypedValue* _rv, Value* uncompressed) asm("_ZN4HPHP12f_nzcompressERKNS_6StringE");

/*
HPHP::Variant HPHP::f_nzuncompress(HPHP::String const&)
_ZN4HPHP14f_nzuncompressERKNS_6StringE

(return value) => rax
_rv => rdi
compressed => rsi
*/

TypedValue* fh_nzuncompress(TypedValue* _rv, Value* compressed) asm("_ZN4HPHP14f_nzuncompressERKNS_6StringE");

/*
HPHP::Variant HPHP::f_lz4compress(HPHP::String const&)
_ZN4HPHP13f_lz4compressERKNS_6StringE

(return value) => rax
_rv => rdi
uncompressed => rsi
*/

TypedValue* fh_lz4compress(TypedValue* _rv, Value* uncompressed) asm("_ZN4HPHP13f_lz4compressERKNS_6StringE");

/*
HPHP::Variant HPHP::f_lz4hccompress(HPHP::String const&)
_ZN4HPHP15f_lz4hccompressERKNS_6StringE

(return value) => rax
_rv => rdi
uncompressed => rsi
*/

TypedValue* fh_lz4hccompress(TypedValue* _rv, Value* uncompressed) asm("_ZN4HPHP15f_lz4hccompressERKNS_6StringE");

/*
HPHP::Variant HPHP::f_lz4uncompress(HPHP::String const&)
_ZN4HPHP15f_lz4uncompressERKNS_6StringE

(return value) => rax
_rv => rdi
compressed => rsi
*/

TypedValue* fh_lz4uncompress(TypedValue* _rv, Value* compressed) asm("_ZN4HPHP15f_lz4uncompressERKNS_6StringE");


} // !HPHP

