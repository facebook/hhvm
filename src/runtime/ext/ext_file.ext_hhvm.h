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
HPHP::Variant HPHP::f_fopen(HPHP::String const&, HPHP::String const&, bool, HPHP::Variant const&)
_ZN4HPHP7f_fopenERKNS_6StringES2_bRKNS_7VariantE

(return value) => rax
_rv => rdi
filename => rsi
mode => rdx
use_include_path => rcx
context => r8
*/

TypedValue* fh_fopen(TypedValue* _rv, Value* filename, Value* mode, bool use_include_path, TypedValue* context) asm("_ZN4HPHP7f_fopenERKNS_6StringES2_bRKNS_7VariantE");

/*
HPHP::Variant HPHP::f_popen(HPHP::String const&, HPHP::String const&)
_ZN4HPHP7f_popenERKNS_6StringES2_

(return value) => rax
_rv => rdi
command => rsi
mode => rdx
*/

TypedValue* fh_popen(TypedValue* _rv, Value* command, Value* mode) asm("_ZN4HPHP7f_popenERKNS_6StringES2_");

/*
bool HPHP::f_fclose(HPHP::Object const&)
_ZN4HPHP8f_fcloseERKNS_6ObjectE

(return value) => rax
handle => rdi
*/

bool fh_fclose(Value* handle) asm("_ZN4HPHP8f_fcloseERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_pclose(HPHP::Object const&)
_ZN4HPHP8f_pcloseERKNS_6ObjectE

(return value) => rax
_rv => rdi
handle => rsi
*/

TypedValue* fh_pclose(TypedValue* _rv, Value* handle) asm("_ZN4HPHP8f_pcloseERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_fseek(HPHP::Object const&, long long, long long)
_ZN4HPHP7f_fseekERKNS_6ObjectExx

(return value) => rax
_rv => rdi
handle => rsi
offset => rdx
whence => rcx
*/

TypedValue* fh_fseek(TypedValue* _rv, Value* handle, long long offset, long long whence) asm("_ZN4HPHP7f_fseekERKNS_6ObjectExx");

/*
bool HPHP::f_rewind(HPHP::Object const&)
_ZN4HPHP8f_rewindERKNS_6ObjectE

(return value) => rax
handle => rdi
*/

bool fh_rewind(Value* handle) asm("_ZN4HPHP8f_rewindERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_ftell(HPHP::Object const&)
_ZN4HPHP7f_ftellERKNS_6ObjectE

(return value) => rax
_rv => rdi
handle => rsi
*/

TypedValue* fh_ftell(TypedValue* _rv, Value* handle) asm("_ZN4HPHP7f_ftellERKNS_6ObjectE");

/*
bool HPHP::f_feof(HPHP::Object const&)
_ZN4HPHP6f_feofERKNS_6ObjectE

(return value) => rax
handle => rdi
*/

bool fh_feof(Value* handle) asm("_ZN4HPHP6f_feofERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_fstat(HPHP::Object const&)
_ZN4HPHP7f_fstatERKNS_6ObjectE

(return value) => rax
_rv => rdi
handle => rsi
*/

TypedValue* fh_fstat(TypedValue* _rv, Value* handle) asm("_ZN4HPHP7f_fstatERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_fread(HPHP::Object const&, long long)
_ZN4HPHP7f_freadERKNS_6ObjectEx

(return value) => rax
_rv => rdi
handle => rsi
length => rdx
*/

TypedValue* fh_fread(TypedValue* _rv, Value* handle, long long length) asm("_ZN4HPHP7f_freadERKNS_6ObjectEx");

/*
HPHP::Variant HPHP::f_fgetc(HPHP::Object const&)
_ZN4HPHP7f_fgetcERKNS_6ObjectE

(return value) => rax
_rv => rdi
handle => rsi
*/

TypedValue* fh_fgetc(TypedValue* _rv, Value* handle) asm("_ZN4HPHP7f_fgetcERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_fgets(HPHP::Object const&, long long)
_ZN4HPHP7f_fgetsERKNS_6ObjectEx

(return value) => rax
_rv => rdi
handle => rsi
length => rdx
*/

TypedValue* fh_fgets(TypedValue* _rv, Value* handle, long long length) asm("_ZN4HPHP7f_fgetsERKNS_6ObjectEx");

/*
HPHP::Variant HPHP::f_fgetss(HPHP::Object const&, long long, HPHP::String const&)
_ZN4HPHP8f_fgetssERKNS_6ObjectExRKNS_6StringE

(return value) => rax
_rv => rdi
handle => rsi
length => rdx
allowable_tags => rcx
*/

TypedValue* fh_fgetss(TypedValue* _rv, Value* handle, long long length, Value* allowable_tags) asm("_ZN4HPHP8f_fgetssERKNS_6ObjectExRKNS_6StringE");

/*
HPHP::Variant HPHP::f_fscanf(int, HPHP::Object const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP8f_fscanfEiRKNS_6ObjectERKNS_6StringERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
handle => rdx
format => rcx
_argv => r8
*/

TypedValue* fh_fscanf(TypedValue* _rv, long long _argc, Value* handle, Value* format, Value* _argv) asm("_ZN4HPHP8f_fscanfEiRKNS_6ObjectERKNS_6StringERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_fpassthru(HPHP::Object const&)
_ZN4HPHP11f_fpassthruERKNS_6ObjectE

(return value) => rax
_rv => rdi
handle => rsi
*/

TypedValue* fh_fpassthru(TypedValue* _rv, Value* handle) asm("_ZN4HPHP11f_fpassthruERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_fwrite(HPHP::Object const&, HPHP::String const&, long long)
_ZN4HPHP8f_fwriteERKNS_6ObjectERKNS_6StringEx

(return value) => rax
_rv => rdi
handle => rsi
data => rdx
length => rcx
*/

TypedValue* fh_fwrite(TypedValue* _rv, Value* handle, Value* data, long long length) asm("_ZN4HPHP8f_fwriteERKNS_6ObjectERKNS_6StringEx");

/*
HPHP::Variant HPHP::f_fputs(HPHP::Object const&, HPHP::String const&, long long)
_ZN4HPHP7f_fputsERKNS_6ObjectERKNS_6StringEx

(return value) => rax
_rv => rdi
handle => rsi
data => rdx
length => rcx
*/

TypedValue* fh_fputs(TypedValue* _rv, Value* handle, Value* data, long long length) asm("_ZN4HPHP7f_fputsERKNS_6ObjectERKNS_6StringEx");

/*
HPHP::Variant HPHP::f_fprintf(int, HPHP::Object const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP9f_fprintfEiRKNS_6ObjectERKNS_6StringERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
handle => rdx
format => rcx
_argv => r8
*/

TypedValue* fh_fprintf(TypedValue* _rv, long long _argc, Value* handle, Value* format, Value* _argv) asm("_ZN4HPHP9f_fprintfEiRKNS_6ObjectERKNS_6StringERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_vfprintf(HPHP::Object const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP10f_vfprintfERKNS_6ObjectERKNS_6StringERKNS_5ArrayE

(return value) => rax
_rv => rdi
handle => rsi
format => rdx
args => rcx
*/

TypedValue* fh_vfprintf(TypedValue* _rv, Value* handle, Value* format, Value* args) asm("_ZN4HPHP10f_vfprintfERKNS_6ObjectERKNS_6StringERKNS_5ArrayE");

/*
bool HPHP::f_fflush(HPHP::Object const&)
_ZN4HPHP8f_fflushERKNS_6ObjectE

(return value) => rax
handle => rdi
*/

bool fh_fflush(Value* handle) asm("_ZN4HPHP8f_fflushERKNS_6ObjectE");

/*
bool HPHP::f_ftruncate(HPHP::Object const&, long long)
_ZN4HPHP11f_ftruncateERKNS_6ObjectEx

(return value) => rax
handle => rdi
size => rsi
*/

bool fh_ftruncate(Value* handle, long long size) asm("_ZN4HPHP11f_ftruncateERKNS_6ObjectEx");

/*
bool HPHP::f_flock(HPHP::Object const&, int, HPHP::VRefParamValue const&)
_ZN4HPHP7f_flockERKNS_6ObjectEiRKNS_14VRefParamValueE

(return value) => rax
handle => rdi
operation => rsi
wouldblock => rdx
*/

bool fh_flock(Value* handle, int operation, TypedValue* wouldblock) asm("_ZN4HPHP7f_flockERKNS_6ObjectEiRKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::f_fputcsv(HPHP::Object const&, HPHP::Array const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP9f_fputcsvERKNS_6ObjectERKNS_5ArrayERKNS_6StringES8_

(return value) => rax
_rv => rdi
handle => rsi
fields => rdx
delimiter => rcx
enclosure => r8
*/

TypedValue* fh_fputcsv(TypedValue* _rv, Value* handle, Value* fields, Value* delimiter, Value* enclosure) asm("_ZN4HPHP9f_fputcsvERKNS_6ObjectERKNS_5ArrayERKNS_6StringES8_");

/*
HPHP::Variant HPHP::f_fgetcsv(HPHP::Object const&, long long, HPHP::String const&, HPHP::String const&)
_ZN4HPHP9f_fgetcsvERKNS_6ObjectExRKNS_6StringES5_

(return value) => rax
_rv => rdi
handle => rsi
length => rdx
delimiter => rcx
enclosure => r8
*/

TypedValue* fh_fgetcsv(TypedValue* _rv, Value* handle, long long length, Value* delimiter, Value* enclosure) asm("_ZN4HPHP9f_fgetcsvERKNS_6ObjectExRKNS_6StringES5_");

/*
HPHP::Variant HPHP::f_file_get_contents(HPHP::String const&, bool, HPHP::Variant const&, long long, long long)
_ZN4HPHP19f_file_get_contentsERKNS_6StringEbRKNS_7VariantExx

(return value) => rax
_rv => rdi
filename => rsi
use_include_path => rdx
context => rcx
offset => r8
maxlen => r9
*/

TypedValue* fh_file_get_contents(TypedValue* _rv, Value* filename, bool use_include_path, TypedValue* context, long long offset, long long maxlen) asm("_ZN4HPHP19f_file_get_contentsERKNS_6StringEbRKNS_7VariantExx");

/*
HPHP::Variant HPHP::f_file_put_contents(HPHP::String const&, HPHP::Variant const&, int, HPHP::Variant const&)
_ZN4HPHP19f_file_put_contentsERKNS_6StringERKNS_7VariantEiS5_

(return value) => rax
_rv => rdi
filename => rsi
data => rdx
flags => rcx
context => r8
*/

TypedValue* fh_file_put_contents(TypedValue* _rv, Value* filename, TypedValue* data, int flags, TypedValue* context) asm("_ZN4HPHP19f_file_put_contentsERKNS_6StringERKNS_7VariantEiS5_");

/*
HPHP::Variant HPHP::f_file(HPHP::String const&, int, HPHP::Variant const&)
_ZN4HPHP6f_fileERKNS_6StringEiRKNS_7VariantE

(return value) => rax
_rv => rdi
filename => rsi
flags => rdx
context => rcx
*/

TypedValue* fh_file(TypedValue* _rv, Value* filename, int flags, TypedValue* context) asm("_ZN4HPHP6f_fileERKNS_6StringEiRKNS_7VariantE");

/*
HPHP::Variant HPHP::f_readfile(HPHP::String const&, bool, HPHP::Variant const&)
_ZN4HPHP10f_readfileERKNS_6StringEbRKNS_7VariantE

(return value) => rax
_rv => rdi
filename => rsi
use_include_path => rdx
context => rcx
*/

TypedValue* fh_readfile(TypedValue* _rv, Value* filename, bool use_include_path, TypedValue* context) asm("_ZN4HPHP10f_readfileERKNS_6StringEbRKNS_7VariantE");

/*
bool HPHP::f_move_uploaded_file(HPHP::String const&, HPHP::String const&)
_ZN4HPHP20f_move_uploaded_fileERKNS_6StringES2_

(return value) => rax
filename => rdi
destination => rsi
*/

bool fh_move_uploaded_file(Value* filename, Value* destination) asm("_ZN4HPHP20f_move_uploaded_fileERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_parse_ini_file(HPHP::String const&, bool, int)
_ZN4HPHP16f_parse_ini_fileERKNS_6StringEbi

(return value) => rax
_rv => rdi
filename => rsi
process_sections => rdx
scanner_mode => rcx
*/

TypedValue* fh_parse_ini_file(TypedValue* _rv, Value* filename, bool process_sections, int scanner_mode) asm("_ZN4HPHP16f_parse_ini_fileERKNS_6StringEbi");

/*
HPHP::Variant HPHP::f_parse_ini_string(HPHP::String const&, bool, int)
_ZN4HPHP18f_parse_ini_stringERKNS_6StringEbi

(return value) => rax
_rv => rdi
ini => rsi
process_sections => rdx
scanner_mode => rcx
*/

TypedValue* fh_parse_ini_string(TypedValue* _rv, Value* ini, bool process_sections, int scanner_mode) asm("_ZN4HPHP18f_parse_ini_stringERKNS_6StringEbi");

/*
HPHP::Variant HPHP::f_parse_hdf_file(HPHP::String const&)
_ZN4HPHP16f_parse_hdf_fileERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_parse_hdf_file(TypedValue* _rv, Value* filename) asm("_ZN4HPHP16f_parse_hdf_fileERKNS_6StringE");

/*
HPHP::Variant HPHP::f_parse_hdf_string(HPHP::String const&)
_ZN4HPHP18f_parse_hdf_stringERKNS_6StringE

(return value) => rax
_rv => rdi
input => rsi
*/

TypedValue* fh_parse_hdf_string(TypedValue* _rv, Value* input) asm("_ZN4HPHP18f_parse_hdf_stringERKNS_6StringE");

/*
bool HPHP::f_write_hdf_file(HPHP::Array const&, HPHP::String const&)
_ZN4HPHP16f_write_hdf_fileERKNS_5ArrayERKNS_6StringE

(return value) => rax
data => rdi
filename => rsi
*/

bool fh_write_hdf_file(Value* data, Value* filename) asm("_ZN4HPHP16f_write_hdf_fileERKNS_5ArrayERKNS_6StringE");

/*
HPHP::String HPHP::f_write_hdf_string(HPHP::Array const&)
_ZN4HPHP18f_write_hdf_stringERKNS_5ArrayE

(return value) => rax
_rv => rdi
data => rsi
*/

Value* fh_write_hdf_string(Value* _rv, Value* data) asm("_ZN4HPHP18f_write_hdf_stringERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_md5_file(HPHP::String const&, bool)
_ZN4HPHP10f_md5_fileERKNS_6StringEb

(return value) => rax
_rv => rdi
filename => rsi
raw_output => rdx
*/

TypedValue* fh_md5_file(TypedValue* _rv, Value* filename, bool raw_output) asm("_ZN4HPHP10f_md5_fileERKNS_6StringEb");

/*
HPHP::Variant HPHP::f_sha1_file(HPHP::String const&, bool)
_ZN4HPHP11f_sha1_fileERKNS_6StringEb

(return value) => rax
_rv => rdi
filename => rsi
raw_output => rdx
*/

TypedValue* fh_sha1_file(TypedValue* _rv, Value* filename, bool raw_output) asm("_ZN4HPHP11f_sha1_fileERKNS_6StringEb");

/*
bool HPHP::f_chmod(HPHP::String const&, long long)
_ZN4HPHP7f_chmodERKNS_6StringEx

(return value) => rax
filename => rdi
mode => rsi
*/

bool fh_chmod(Value* filename, long long mode) asm("_ZN4HPHP7f_chmodERKNS_6StringEx");

/*
bool HPHP::f_chown(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP7f_chownERKNS_6StringERKNS_7VariantE

(return value) => rax
filename => rdi
user => rsi
*/

bool fh_chown(Value* filename, TypedValue* user) asm("_ZN4HPHP7f_chownERKNS_6StringERKNS_7VariantE");

/*
bool HPHP::f_lchown(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP8f_lchownERKNS_6StringERKNS_7VariantE

(return value) => rax
filename => rdi
user => rsi
*/

bool fh_lchown(Value* filename, TypedValue* user) asm("_ZN4HPHP8f_lchownERKNS_6StringERKNS_7VariantE");

/*
bool HPHP::f_chgrp(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP7f_chgrpERKNS_6StringERKNS_7VariantE

(return value) => rax
filename => rdi
group => rsi
*/

bool fh_chgrp(Value* filename, TypedValue* group) asm("_ZN4HPHP7f_chgrpERKNS_6StringERKNS_7VariantE");

/*
bool HPHP::f_lchgrp(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP8f_lchgrpERKNS_6StringERKNS_7VariantE

(return value) => rax
filename => rdi
group => rsi
*/

bool fh_lchgrp(Value* filename, TypedValue* group) asm("_ZN4HPHP8f_lchgrpERKNS_6StringERKNS_7VariantE");

/*
bool HPHP::f_touch(HPHP::String const&, long long, long long)
_ZN4HPHP7f_touchERKNS_6StringExx

(return value) => rax
filename => rdi
mtime => rsi
atime => rdx
*/

bool fh_touch(Value* filename, long long mtime, long long atime) asm("_ZN4HPHP7f_touchERKNS_6StringExx");

/*
bool HPHP::f_copy(HPHP::String const&, HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP6f_copyERKNS_6StringES2_RKNS_7VariantE

(return value) => rax
source => rdi
dest => rsi
context => rdx
*/

bool fh_copy(Value* source, Value* dest, TypedValue* context) asm("_ZN4HPHP6f_copyERKNS_6StringES2_RKNS_7VariantE");

/*
bool HPHP::f_rename(HPHP::String const&, HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP8f_renameERKNS_6StringES2_RKNS_7VariantE

(return value) => rax
oldname => rdi
newname => rsi
context => rdx
*/

bool fh_rename(Value* oldname, Value* newname, TypedValue* context) asm("_ZN4HPHP8f_renameERKNS_6StringES2_RKNS_7VariantE");

/*
long long HPHP::f_umask(HPHP::Variant const&)
_ZN4HPHP7f_umaskERKNS_7VariantE

(return value) => rax
mask => rdi
*/

long long fh_umask(TypedValue* mask) asm("_ZN4HPHP7f_umaskERKNS_7VariantE");

/*
bool HPHP::f_unlink(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP8f_unlinkERKNS_6StringERKNS_7VariantE

(return value) => rax
filename => rdi
context => rsi
*/

bool fh_unlink(Value* filename, TypedValue* context) asm("_ZN4HPHP8f_unlinkERKNS_6StringERKNS_7VariantE");

/*
bool HPHP::f_link(HPHP::String const&, HPHP::String const&)
_ZN4HPHP6f_linkERKNS_6StringES2_

(return value) => rax
target => rdi
link => rsi
*/

bool fh_link(Value* target, Value* link) asm("_ZN4HPHP6f_linkERKNS_6StringES2_");

/*
bool HPHP::f_symlink(HPHP::String const&, HPHP::String const&)
_ZN4HPHP9f_symlinkERKNS_6StringES2_

(return value) => rax
target => rdi
link => rsi
*/

bool fh_symlink(Value* target, Value* link) asm("_ZN4HPHP9f_symlinkERKNS_6StringES2_");

/*
HPHP::String HPHP::f_basename(HPHP::String const&, HPHP::String const&)
_ZN4HPHP10f_basenameERKNS_6StringES2_

(return value) => rax
_rv => rdi
path => rsi
suffix => rdx
*/

Value* fh_basename(Value* _rv, Value* path, Value* suffix) asm("_ZN4HPHP10f_basenameERKNS_6StringES2_");

/*
bool HPHP::f_fnmatch(HPHP::String const&, HPHP::String const&, int)
_ZN4HPHP9f_fnmatchERKNS_6StringES2_i

(return value) => rax
pattern => rdi
filename => rsi
flags => rdx
*/

bool fh_fnmatch(Value* pattern, Value* filename, int flags) asm("_ZN4HPHP9f_fnmatchERKNS_6StringES2_i");

/*
HPHP::Variant HPHP::f_glob(HPHP::String const&, int)
_ZN4HPHP6f_globERKNS_6StringEi

(return value) => rax
_rv => rdi
pattern => rsi
flags => rdx
*/

TypedValue* fh_glob(TypedValue* _rv, Value* pattern, int flags) asm("_ZN4HPHP6f_globERKNS_6StringEi");

/*
HPHP::Variant HPHP::f_tempnam(HPHP::String const&, HPHP::String const&)
_ZN4HPHP9f_tempnamERKNS_6StringES2_

(return value) => rax
_rv => rdi
dir => rsi
prefix => rdx
*/

TypedValue* fh_tempnam(TypedValue* _rv, Value* dir, Value* prefix) asm("_ZN4HPHP9f_tempnamERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_tmpfile()
_ZN4HPHP9f_tmpfileEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_tmpfile(TypedValue* _rv) asm("_ZN4HPHP9f_tmpfileEv");

/*
HPHP::Variant HPHP::f_fileperms(HPHP::String const&)
_ZN4HPHP11f_filepermsERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_fileperms(TypedValue* _rv, Value* filename) asm("_ZN4HPHP11f_filepermsERKNS_6StringE");

/*
HPHP::Variant HPHP::f_fileinode(HPHP::String const&)
_ZN4HPHP11f_fileinodeERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_fileinode(TypedValue* _rv, Value* filename) asm("_ZN4HPHP11f_fileinodeERKNS_6StringE");

/*
HPHP::Variant HPHP::f_filesize(HPHP::String const&)
_ZN4HPHP10f_filesizeERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_filesize(TypedValue* _rv, Value* filename) asm("_ZN4HPHP10f_filesizeERKNS_6StringE");

/*
HPHP::Variant HPHP::f_fileowner(HPHP::String const&)
_ZN4HPHP11f_fileownerERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_fileowner(TypedValue* _rv, Value* filename) asm("_ZN4HPHP11f_fileownerERKNS_6StringE");

/*
HPHP::Variant HPHP::f_filegroup(HPHP::String const&)
_ZN4HPHP11f_filegroupERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_filegroup(TypedValue* _rv, Value* filename) asm("_ZN4HPHP11f_filegroupERKNS_6StringE");

/*
HPHP::Variant HPHP::f_fileatime(HPHP::String const&)
_ZN4HPHP11f_fileatimeERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_fileatime(TypedValue* _rv, Value* filename) asm("_ZN4HPHP11f_fileatimeERKNS_6StringE");

/*
HPHP::Variant HPHP::f_filemtime(HPHP::String const&)
_ZN4HPHP11f_filemtimeERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_filemtime(TypedValue* _rv, Value* filename) asm("_ZN4HPHP11f_filemtimeERKNS_6StringE");

/*
HPHP::Variant HPHP::f_filectime(HPHP::String const&)
_ZN4HPHP11f_filectimeERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_filectime(TypedValue* _rv, Value* filename) asm("_ZN4HPHP11f_filectimeERKNS_6StringE");

/*
HPHP::Variant HPHP::f_filetype(HPHP::String const&)
_ZN4HPHP10f_filetypeERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_filetype(TypedValue* _rv, Value* filename) asm("_ZN4HPHP10f_filetypeERKNS_6StringE");

/*
HPHP::Variant HPHP::f_linkinfo(HPHP::String const&)
_ZN4HPHP10f_linkinfoERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_linkinfo(TypedValue* _rv, Value* filename) asm("_ZN4HPHP10f_linkinfoERKNS_6StringE");

/*
bool HPHP::f_is_writable(HPHP::String const&)
_ZN4HPHP13f_is_writableERKNS_6StringE

(return value) => rax
filename => rdi
*/

bool fh_is_writable(Value* filename) asm("_ZN4HPHP13f_is_writableERKNS_6StringE");

/*
bool HPHP::f_is_writeable(HPHP::String const&)
_ZN4HPHP14f_is_writeableERKNS_6StringE

(return value) => rax
filename => rdi
*/

bool fh_is_writeable(Value* filename) asm("_ZN4HPHP14f_is_writeableERKNS_6StringE");

/*
bool HPHP::f_is_readable(HPHP::String const&)
_ZN4HPHP13f_is_readableERKNS_6StringE

(return value) => rax
filename => rdi
*/

bool fh_is_readable(Value* filename) asm("_ZN4HPHP13f_is_readableERKNS_6StringE");

/*
bool HPHP::f_is_executable(HPHP::String const&)
_ZN4HPHP15f_is_executableERKNS_6StringE

(return value) => rax
filename => rdi
*/

bool fh_is_executable(Value* filename) asm("_ZN4HPHP15f_is_executableERKNS_6StringE");

/*
bool HPHP::f_is_file(HPHP::String const&)
_ZN4HPHP9f_is_fileERKNS_6StringE

(return value) => rax
filename => rdi
*/

bool fh_is_file(Value* filename) asm("_ZN4HPHP9f_is_fileERKNS_6StringE");

/*
bool HPHP::f_is_dir(HPHP::String const&)
_ZN4HPHP8f_is_dirERKNS_6StringE

(return value) => rax
filename => rdi
*/

bool fh_is_dir(Value* filename) asm("_ZN4HPHP8f_is_dirERKNS_6StringE");

/*
bool HPHP::f_is_link(HPHP::String const&)
_ZN4HPHP9f_is_linkERKNS_6StringE

(return value) => rax
filename => rdi
*/

bool fh_is_link(Value* filename) asm("_ZN4HPHP9f_is_linkERKNS_6StringE");

/*
bool HPHP::f_is_uploaded_file(HPHP::String const&)
_ZN4HPHP18f_is_uploaded_fileERKNS_6StringE

(return value) => rax
filename => rdi
*/

bool fh_is_uploaded_file(Value* filename) asm("_ZN4HPHP18f_is_uploaded_fileERKNS_6StringE");

/*
bool HPHP::f_file_exists(HPHP::String const&)
_ZN4HPHP13f_file_existsERKNS_6StringE

(return value) => rax
filename => rdi
*/

bool fh_file_exists(Value* filename) asm("_ZN4HPHP13f_file_existsERKNS_6StringE");

/*
HPHP::Variant HPHP::f_stat(HPHP::String const&)
_ZN4HPHP6f_statERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_stat(TypedValue* _rv, Value* filename) asm("_ZN4HPHP6f_statERKNS_6StringE");

/*
HPHP::Variant HPHP::f_lstat(HPHP::String const&)
_ZN4HPHP7f_lstatERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_lstat(TypedValue* _rv, Value* filename) asm("_ZN4HPHP7f_lstatERKNS_6StringE");

/*
void HPHP::f_clearstatcache()
_ZN4HPHP16f_clearstatcacheEv

*/

void fh_clearstatcache() asm("_ZN4HPHP16f_clearstatcacheEv");

/*
HPHP::Variant HPHP::f_readlink(HPHP::String const&)
_ZN4HPHP10f_readlinkERKNS_6StringE

(return value) => rax
_rv => rdi
path => rsi
*/

TypedValue* fh_readlink(TypedValue* _rv, Value* path) asm("_ZN4HPHP10f_readlinkERKNS_6StringE");

/*
HPHP::Variant HPHP::f_realpath(HPHP::String const&)
_ZN4HPHP10f_realpathERKNS_6StringE

(return value) => rax
_rv => rdi
path => rsi
*/

TypedValue* fh_realpath(TypedValue* _rv, Value* path) asm("_ZN4HPHP10f_realpathERKNS_6StringE");

/*
HPHP::Variant HPHP::f_pathinfo(HPHP::String const&, int)
_ZN4HPHP10f_pathinfoERKNS_6StringEi

(return value) => rax
_rv => rdi
path => rsi
opt => rdx
*/

TypedValue* fh_pathinfo(TypedValue* _rv, Value* path, int opt) asm("_ZN4HPHP10f_pathinfoERKNS_6StringEi");

/*
HPHP::Variant HPHP::f_disk_free_space(HPHP::String const&)
_ZN4HPHP17f_disk_free_spaceERKNS_6StringE

(return value) => rax
_rv => rdi
directory => rsi
*/

TypedValue* fh_disk_free_space(TypedValue* _rv, Value* directory) asm("_ZN4HPHP17f_disk_free_spaceERKNS_6StringE");

/*
HPHP::Variant HPHP::f_diskfreespace(HPHP::String const&)
_ZN4HPHP15f_diskfreespaceERKNS_6StringE

(return value) => rax
_rv => rdi
directory => rsi
*/

TypedValue* fh_diskfreespace(TypedValue* _rv, Value* directory) asm("_ZN4HPHP15f_diskfreespaceERKNS_6StringE");

/*
HPHP::Variant HPHP::f_disk_total_space(HPHP::String const&)
_ZN4HPHP18f_disk_total_spaceERKNS_6StringE

(return value) => rax
_rv => rdi
directory => rsi
*/

TypedValue* fh_disk_total_space(TypedValue* _rv, Value* directory) asm("_ZN4HPHP18f_disk_total_spaceERKNS_6StringE");

/*
bool HPHP::f_mkdir(HPHP::String const&, long long, bool, HPHP::Variant const&)
_ZN4HPHP7f_mkdirERKNS_6StringExbRKNS_7VariantE

(return value) => rax
pathname => rdi
mode => rsi
recursive => rdx
context => rcx
*/

bool fh_mkdir(Value* pathname, long long mode, bool recursive, TypedValue* context) asm("_ZN4HPHP7f_mkdirERKNS_6StringExbRKNS_7VariantE");

/*
bool HPHP::f_rmdir(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP7f_rmdirERKNS_6StringERKNS_7VariantE

(return value) => rax
dirname => rdi
context => rsi
*/

bool fh_rmdir(Value* dirname, TypedValue* context) asm("_ZN4HPHP7f_rmdirERKNS_6StringERKNS_7VariantE");

/*
HPHP::String HPHP::f_dirname(HPHP::String const&)
_ZN4HPHP9f_dirnameERKNS_6StringE

(return value) => rax
_rv => rdi
path => rsi
*/

Value* fh_dirname(Value* _rv, Value* path) asm("_ZN4HPHP9f_dirnameERKNS_6StringE");

/*
HPHP::Variant HPHP::f_getcwd()
_ZN4HPHP8f_getcwdEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_getcwd(TypedValue* _rv) asm("_ZN4HPHP8f_getcwdEv");

/*
bool HPHP::f_chdir(HPHP::String const&)
_ZN4HPHP7f_chdirERKNS_6StringE

(return value) => rax
directory => rdi
*/

bool fh_chdir(Value* directory) asm("_ZN4HPHP7f_chdirERKNS_6StringE");

/*
bool HPHP::f_chroot(HPHP::String const&)
_ZN4HPHP8f_chrootERKNS_6StringE

(return value) => rax
directory => rdi
*/

bool fh_chroot(Value* directory) asm("_ZN4HPHP8f_chrootERKNS_6StringE");

/*
HPHP::Variant HPHP::f_dir(HPHP::String const&)
_ZN4HPHP5f_dirERKNS_6StringE

(return value) => rax
_rv => rdi
directory => rsi
*/

TypedValue* fh_dir(TypedValue* _rv, Value* directory) asm("_ZN4HPHP5f_dirERKNS_6StringE");

/*
HPHP::Variant HPHP::f_opendir(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP9f_opendirERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
path => rsi
context => rdx
*/

TypedValue* fh_opendir(TypedValue* _rv, Value* path, TypedValue* context) asm("_ZN4HPHP9f_opendirERKNS_6StringERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_readdir(HPHP::Object const&)
_ZN4HPHP9f_readdirERKNS_6ObjectE

(return value) => rax
_rv => rdi
dir_handle => rsi
*/

TypedValue* fh_readdir(TypedValue* _rv, Value* dir_handle) asm("_ZN4HPHP9f_readdirERKNS_6ObjectE");

/*
void HPHP::f_rewinddir(HPHP::Object const&)
_ZN4HPHP11f_rewinddirERKNS_6ObjectE

dir_handle => rdi
*/

void fh_rewinddir(Value* dir_handle) asm("_ZN4HPHP11f_rewinddirERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_scandir(HPHP::String const&, bool, HPHP::Variant const&)
_ZN4HPHP9f_scandirERKNS_6StringEbRKNS_7VariantE

(return value) => rax
_rv => rdi
directory => rsi
descending => rdx
context => rcx
*/

TypedValue* fh_scandir(TypedValue* _rv, Value* directory, bool descending, TypedValue* context) asm("_ZN4HPHP9f_scandirERKNS_6StringEbRKNS_7VariantE");

/*
void HPHP::f_closedir(HPHP::Object const&)
_ZN4HPHP10f_closedirERKNS_6ObjectE

dir_handle => rdi
*/

void fh_closedir(Value* dir_handle) asm("_ZN4HPHP10f_closedirERKNS_6ObjectE");


} // !HPHP

