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

TypedValue* fh_fopen(TypedValue* _rv, Value* filename, Value* mode, bool use_include_path, TypedValue* context) asm("_ZN4HPHP7f_fopenERKNS_6StringES2_bRKNS_7VariantE");

TypedValue* fh_popen(TypedValue* _rv, Value* command, Value* mode) asm("_ZN4HPHP7f_popenERKNS_6StringES2_");

bool fh_fclose(Value* handle) asm("_ZN4HPHP8f_fcloseERKNS_6ObjectE");

TypedValue* fh_pclose(TypedValue* _rv, Value* handle) asm("_ZN4HPHP8f_pcloseERKNS_6ObjectE");

TypedValue* fh_fseek(TypedValue* _rv, Value* handle, long offset, long whence) asm("_ZN4HPHP7f_fseekERKNS_6ObjectEll");

bool fh_rewind(Value* handle) asm("_ZN4HPHP8f_rewindERKNS_6ObjectE");

TypedValue* fh_ftell(TypedValue* _rv, Value* handle) asm("_ZN4HPHP7f_ftellERKNS_6ObjectE");

bool fh_feof(Value* handle) asm("_ZN4HPHP6f_feofERKNS_6ObjectE");

TypedValue* fh_fstat(TypedValue* _rv, Value* handle) asm("_ZN4HPHP7f_fstatERKNS_6ObjectE");

TypedValue* fh_fread(TypedValue* _rv, Value* handle, long length) asm("_ZN4HPHP7f_freadERKNS_6ObjectEl");

TypedValue* fh_fgetc(TypedValue* _rv, Value* handle) asm("_ZN4HPHP7f_fgetcERKNS_6ObjectE");

TypedValue* fh_fgets(TypedValue* _rv, Value* handle, long length) asm("_ZN4HPHP7f_fgetsERKNS_6ObjectEl");

TypedValue* fh_fgetss(TypedValue* _rv, Value* handle, long length, Value* allowable_tags) asm("_ZN4HPHP8f_fgetssERKNS_6ObjectElRKNS_6StringE");

TypedValue* fh_fscanf(TypedValue* _rv, int64_t _argc, Value* handle, Value* format, Value* _argv) asm("_ZN4HPHP8f_fscanfEiRKNS_6ObjectERKNS_6StringERKNS_5ArrayE");

TypedValue* fh_fpassthru(TypedValue* _rv, Value* handle) asm("_ZN4HPHP11f_fpassthruERKNS_6ObjectE");

TypedValue* fh_fwrite(TypedValue* _rv, Value* handle, Value* data, long length) asm("_ZN4HPHP8f_fwriteERKNS_6ObjectERKNS_6StringEl");

TypedValue* fh_fputs(TypedValue* _rv, Value* handle, Value* data, long length) asm("_ZN4HPHP7f_fputsERKNS_6ObjectERKNS_6StringEl");

TypedValue* fh_fprintf(TypedValue* _rv, int64_t _argc, Value* handle, Value* format, Value* _argv) asm("_ZN4HPHP9f_fprintfEiRKNS_6ObjectERKNS_6StringERKNS_5ArrayE");

TypedValue* fh_vfprintf(TypedValue* _rv, Value* handle, Value* format, Value* args) asm("_ZN4HPHP10f_vfprintfERKNS_6ObjectERKNS_6StringERKNS_5ArrayE");

bool fh_fflush(Value* handle) asm("_ZN4HPHP8f_fflushERKNS_6ObjectE");

bool fh_ftruncate(Value* handle, long size) asm("_ZN4HPHP11f_ftruncateERKNS_6ObjectEl");

bool fh_flock(Value* handle, int operation, TypedValue* wouldblock) asm("_ZN4HPHP7f_flockERKNS_6ObjectEiRKNS_14VRefParamValueE");

TypedValue* fh_fputcsv(TypedValue* _rv, Value* handle, Value* fields, Value* delimiter, Value* enclosure) asm("_ZN4HPHP9f_fputcsvERKNS_6ObjectERKNS_5ArrayERKNS_6StringES8_");

TypedValue* fh_fgetcsv(TypedValue* _rv, Value* handle, long length, Value* delimiter, Value* enclosure) asm("_ZN4HPHP9f_fgetcsvERKNS_6ObjectElRKNS_6StringES5_");

TypedValue* fh_file_get_contents(TypedValue* _rv, Value* filename, bool use_include_path, TypedValue* context, long offset, long maxlen) asm("_ZN4HPHP19f_file_get_contentsERKNS_6StringEbRKNS_7VariantEll");

TypedValue* fh_file_put_contents(TypedValue* _rv, Value* filename, TypedValue* data, int flags, TypedValue* context) asm("_ZN4HPHP19f_file_put_contentsERKNS_6StringERKNS_7VariantEiS5_");

TypedValue* fh_file(TypedValue* _rv, Value* filename, int flags, TypedValue* context) asm("_ZN4HPHP6f_fileERKNS_6StringEiRKNS_7VariantE");

TypedValue* fh_readfile(TypedValue* _rv, Value* filename, bool use_include_path, TypedValue* context) asm("_ZN4HPHP10f_readfileERKNS_6StringEbRKNS_7VariantE");

bool fh_move_uploaded_file(Value* filename, Value* destination) asm("_ZN4HPHP20f_move_uploaded_fileERKNS_6StringES2_");

TypedValue* fh_parse_ini_file(TypedValue* _rv, Value* filename, bool process_sections, int scanner_mode) asm("_ZN4HPHP16f_parse_ini_fileERKNS_6StringEbi");

bool fh_file_exists(Value* filename) asm("_ZN4HPHP13f_file_existsERKNS_6StringE");

TypedValue* fh_parse_ini_string(TypedValue* _rv, Value* ini, bool process_sections, int scanner_mode) asm("_ZN4HPHP18f_parse_ini_stringERKNS_6StringEbi");

TypedValue* fh_parse_hdf_file(TypedValue* _rv, Value* filename) asm("_ZN4HPHP16f_parse_hdf_fileERKNS_6StringE");

TypedValue* fh_parse_hdf_string(TypedValue* _rv, Value* input) asm("_ZN4HPHP18f_parse_hdf_stringERKNS_6StringE");

bool fh_write_hdf_file(Value* data, Value* filename) asm("_ZN4HPHP16f_write_hdf_fileERKNS_5ArrayERKNS_6StringE");

Value* fh_write_hdf_string(Value* _rv, Value* data) asm("_ZN4HPHP18f_write_hdf_stringERKNS_5ArrayE");

TypedValue* fh_md5_file(TypedValue* _rv, Value* filename, bool raw_output) asm("_ZN4HPHP10f_md5_fileERKNS_6StringEb");

TypedValue* fh_sha1_file(TypedValue* _rv, Value* filename, bool raw_output) asm("_ZN4HPHP11f_sha1_fileERKNS_6StringEb");

TypedValue* fh_fileperms(TypedValue* _rv, Value* filename) asm("_ZN4HPHP11f_filepermsERKNS_6StringE");

TypedValue* fh_fileinode(TypedValue* _rv, Value* filename) asm("_ZN4HPHP11f_fileinodeERKNS_6StringE");

TypedValue* fh_filesize(TypedValue* _rv, Value* filename) asm("_ZN4HPHP10f_filesizeERKNS_6StringE");

TypedValue* fh_fileowner(TypedValue* _rv, Value* filename) asm("_ZN4HPHP11f_fileownerERKNS_6StringE");

TypedValue* fh_filegroup(TypedValue* _rv, Value* filename) asm("_ZN4HPHP11f_filegroupERKNS_6StringE");

TypedValue* fh_fileatime(TypedValue* _rv, Value* filename) asm("_ZN4HPHP11f_fileatimeERKNS_6StringE");

TypedValue* fh_filemtime(TypedValue* _rv, Value* filename) asm("_ZN4HPHP11f_filemtimeERKNS_6StringE");

TypedValue* fh_filectime(TypedValue* _rv, Value* filename) asm("_ZN4HPHP11f_filectimeERKNS_6StringE");

TypedValue* fh_filetype(TypedValue* _rv, Value* filename) asm("_ZN4HPHP10f_filetypeERKNS_6StringE");

TypedValue* fh_linkinfo(TypedValue* _rv, Value* filename) asm("_ZN4HPHP10f_linkinfoERKNS_6StringE");

bool fh_is_writable(Value* filename) asm("_ZN4HPHP13f_is_writableERKNS_6StringE");

bool fh_is_writeable(Value* filename) asm("_ZN4HPHP14f_is_writeableERKNS_6StringE");

bool fh_is_readable(Value* filename) asm("_ZN4HPHP13f_is_readableERKNS_6StringE");

bool fh_is_executable(Value* filename) asm("_ZN4HPHP15f_is_executableERKNS_6StringE");

bool fh_is_file(Value* filename) asm("_ZN4HPHP9f_is_fileERKNS_6StringE");

bool fh_is_dir(Value* filename) asm("_ZN4HPHP8f_is_dirERKNS_6StringE");

bool fh_is_link(Value* filename) asm("_ZN4HPHP9f_is_linkERKNS_6StringE");

bool fh_is_uploaded_file(Value* filename) asm("_ZN4HPHP18f_is_uploaded_fileERKNS_6StringE");

TypedValue* fh_stat(TypedValue* _rv, Value* filename) asm("_ZN4HPHP6f_statERKNS_6StringE");

TypedValue* fh_lstat(TypedValue* _rv, Value* filename) asm("_ZN4HPHP7f_lstatERKNS_6StringE");

void fh_clearstatcache() asm("_ZN4HPHP16f_clearstatcacheEv");

TypedValue* fh_readlink(TypedValue* _rv, Value* path) asm("_ZN4HPHP10f_readlinkERKNS_6StringE");

TypedValue* fh_realpath(TypedValue* _rv, Value* path) asm("_ZN4HPHP10f_realpathERKNS_6StringE");

TypedValue* fh_pathinfo(TypedValue* _rv, Value* path, int opt) asm("_ZN4HPHP10f_pathinfoERKNS_6StringEi");

Value* fh_dirname(Value* _rv, Value* path) asm("_ZN4HPHP9f_dirnameERKNS_6StringE");

Value* fh_basename(Value* _rv, Value* path, Value* suffix) asm("_ZN4HPHP10f_basenameERKNS_6StringES2_");

TypedValue* fh_disk_free_space(TypedValue* _rv, Value* directory) asm("_ZN4HPHP17f_disk_free_spaceERKNS_6StringE");

TypedValue* fh_diskfreespace(TypedValue* _rv, Value* directory) asm("_ZN4HPHP15f_diskfreespaceERKNS_6StringE");

TypedValue* fh_disk_total_space(TypedValue* _rv, Value* directory) asm("_ZN4HPHP18f_disk_total_spaceERKNS_6StringE");

bool fh_chmod(Value* filename, long mode) asm("_ZN4HPHP7f_chmodERKNS_6StringEl");

bool fh_chown(Value* filename, TypedValue* user) asm("_ZN4HPHP7f_chownERKNS_6StringERKNS_7VariantE");

bool fh_lchown(Value* filename, TypedValue* user) asm("_ZN4HPHP8f_lchownERKNS_6StringERKNS_7VariantE");

bool fh_chgrp(Value* filename, TypedValue* group) asm("_ZN4HPHP7f_chgrpERKNS_6StringERKNS_7VariantE");

bool fh_lchgrp(Value* filename, TypedValue* group) asm("_ZN4HPHP8f_lchgrpERKNS_6StringERKNS_7VariantE");

bool fh_touch(Value* filename, long mtime, long atime) asm("_ZN4HPHP7f_touchERKNS_6StringEll");

bool fh_copy(Value* source, Value* dest, TypedValue* context) asm("_ZN4HPHP6f_copyERKNS_6StringES2_RKNS_7VariantE");

bool fh_rename(Value* oldname, Value* newname, TypedValue* context) asm("_ZN4HPHP8f_renameERKNS_6StringES2_RKNS_7VariantE");

long fh_umask(TypedValue* mask) asm("_ZN4HPHP7f_umaskERKNS_7VariantE");

bool fh_unlink(Value* filename, TypedValue* context) asm("_ZN4HPHP8f_unlinkERKNS_6StringERKNS_7VariantE");

bool fh_link(Value* target, Value* link) asm("_ZN4HPHP6f_linkERKNS_6StringES2_");

bool fh_symlink(Value* target, Value* link) asm("_ZN4HPHP9f_symlinkERKNS_6StringES2_");

bool fh_fnmatch(Value* pattern, Value* filename, int flags) asm("_ZN4HPHP9f_fnmatchERKNS_6StringES2_i");

TypedValue* fh_glob(TypedValue* _rv, Value* pattern, int flags) asm("_ZN4HPHP6f_globERKNS_6StringEi");

TypedValue* fh_tempnam(TypedValue* _rv, Value* dir, Value* prefix) asm("_ZN4HPHP9f_tempnamERKNS_6StringES2_");

TypedValue* fh_tmpfile(TypedValue* _rv) asm("_ZN4HPHP9f_tmpfileEv");

bool fh_mkdir(Value* pathname, long mode, bool recursive, TypedValue* context) asm("_ZN4HPHP7f_mkdirERKNS_6StringElbRKNS_7VariantE");

bool fh_rmdir(Value* dirname, TypedValue* context) asm("_ZN4HPHP7f_rmdirERKNS_6StringERKNS_7VariantE");

TypedValue* fh_getcwd(TypedValue* _rv) asm("_ZN4HPHP8f_getcwdEv");

bool fh_chdir(Value* directory) asm("_ZN4HPHP7f_chdirERKNS_6StringE");

bool fh_chroot(Value* directory) asm("_ZN4HPHP8f_chrootERKNS_6StringE");

TypedValue* fh_dir(TypedValue* _rv, Value* directory) asm("_ZN4HPHP5f_dirERKNS_6StringE");

TypedValue* fh_opendir(TypedValue* _rv, Value* path, TypedValue* context) asm("_ZN4HPHP9f_opendirERKNS_6StringERKNS_7VariantE");

TypedValue* fh_readdir(TypedValue* _rv, Value* dir_handle) asm("_ZN4HPHP9f_readdirERKNS_6ObjectE");

void fh_rewinddir(Value* dir_handle) asm("_ZN4HPHP11f_rewinddirERKNS_6ObjectE");

TypedValue* fh_scandir(TypedValue* _rv, Value* directory, bool descending, TypedValue* context) asm("_ZN4HPHP9f_scandirERKNS_6StringEbRKNS_7VariantE");

void fh_closedir(Value* dir_handle) asm("_ZN4HPHP10f_closedirERKNS_6ObjectE");

} // namespace HPHP
