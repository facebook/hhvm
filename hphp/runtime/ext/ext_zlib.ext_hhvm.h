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

TypedValue* fh_readgzfile(TypedValue* _rv, Value* filename, bool use_include_path) asm("_ZN4HPHP12f_readgzfileERKNS_6StringEb");

Value* fh_gzopen(Value* _rv, Value* filename, Value* mode, bool use_include_path) asm("_ZN4HPHP8f_gzopenERKNS_6StringES2_b");

TypedValue* fh_gzpassthru(TypedValue* _rv, Value* zp) asm("_ZN4HPHP12f_gzpassthruERKNS_6ObjectE");

TypedValue* fh_gzfile(TypedValue* _rv, Value* filename, bool use_include_path) asm("_ZN4HPHP8f_gzfileERKNS_6StringEb");

TypedValue* fh_gzgets(TypedValue* _rv, Value* zp, long length) asm("_ZN4HPHP8f_gzgetsERKNS_6ObjectEl");

TypedValue* fh_gzcompress(TypedValue* _rv, Value* data, int level) asm("_ZN4HPHP12f_gzcompressERKNS_6StringEi");

TypedValue* fh_gzuncompress(TypedValue* _rv, Value* data, int limit) asm("_ZN4HPHP14f_gzuncompressERKNS_6StringEi");

TypedValue* fh_gzdeflate(TypedValue* _rv, Value* data, int level) asm("_ZN4HPHP11f_gzdeflateERKNS_6StringEi");

TypedValue* fh_gzinflate(TypedValue* _rv, Value* data, int limit) asm("_ZN4HPHP11f_gzinflateERKNS_6StringEi");

TypedValue* fh_gzencode(TypedValue* _rv, Value* data, int level, int encoding_mode) asm("_ZN4HPHP10f_gzencodeERKNS_6StringEii");

TypedValue* fh_gzdecode(TypedValue* _rv, Value* data) asm("_ZN4HPHP10f_gzdecodeERKNS_6StringE");

Value* fh_zlib_get_coding_type(Value* _rv) asm("_ZN4HPHP22f_zlib_get_coding_typeEv");

bool fh_gzclose(Value* zp) asm("_ZN4HPHP9f_gzcloseERKNS_6ObjectE");

TypedValue* fh_gzread(TypedValue* _rv, Value* zp, long length) asm("_ZN4HPHP8f_gzreadERKNS_6ObjectEl");

TypedValue* fh_gzseek(TypedValue* _rv, Value* zp, long offset, long whence) asm("_ZN4HPHP8f_gzseekERKNS_6ObjectEll");

TypedValue* fh_gztell(TypedValue* _rv, Value* zp) asm("_ZN4HPHP8f_gztellERKNS_6ObjectE");

bool fh_gzeof(Value* zp) asm("_ZN4HPHP7f_gzeofERKNS_6ObjectE");

bool fh_gzrewind(Value* zp) asm("_ZN4HPHP10f_gzrewindERKNS_6ObjectE");

TypedValue* fh_gzgetc(TypedValue* _rv, Value* zp) asm("_ZN4HPHP8f_gzgetcERKNS_6ObjectE");

TypedValue* fh_gzgetss(TypedValue* _rv, Value* zp, long length, Value* allowable_tags) asm("_ZN4HPHP9f_gzgetssERKNS_6ObjectElRKNS_6StringE");

TypedValue* fh_gzputs(TypedValue* _rv, Value* zp, Value* str, long length) asm("_ZN4HPHP8f_gzputsERKNS_6ObjectERKNS_6StringEl");

TypedValue* fh_gzwrite(TypedValue* _rv, Value* zp, Value* str, long length) asm("_ZN4HPHP9f_gzwriteERKNS_6ObjectERKNS_6StringEl");

TypedValue* fh_qlzcompress(TypedValue* _rv, Value* data, int level) asm("_ZN4HPHP13f_qlzcompressERKNS_6StringEi");

TypedValue* fh_qlzuncompress(TypedValue* _rv, Value* data, int level) asm("_ZN4HPHP15f_qlzuncompressERKNS_6StringEi");

TypedValue* fh_sncompress(TypedValue* _rv, Value* data) asm("_ZN4HPHP12f_sncompressERKNS_6StringE");

TypedValue* fh_snuncompress(TypedValue* _rv, Value* data) asm("_ZN4HPHP14f_snuncompressERKNS_6StringE");

TypedValue* fh_nzcompress(TypedValue* _rv, Value* uncompressed) asm("_ZN4HPHP12f_nzcompressERKNS_6StringE");

TypedValue* fh_nzuncompress(TypedValue* _rv, Value* compressed) asm("_ZN4HPHP14f_nzuncompressERKNS_6StringE");

TypedValue* fh_lz4compress(TypedValue* _rv, Value* uncompressed) asm("_ZN4HPHP13f_lz4compressERKNS_6StringE");

TypedValue* fh_lz4hccompress(TypedValue* _rv, Value* uncompressed) asm("_ZN4HPHP15f_lz4hccompressERKNS_6StringE");

TypedValue* fh_lz4uncompress(TypedValue* _rv, Value* compressed) asm("_ZN4HPHP15f_lz4uncompressERKNS_6StringE");

} // namespace HPHP
