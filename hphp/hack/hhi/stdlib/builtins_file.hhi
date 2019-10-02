<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

/* HH_FIXME[4110] */
const resource STDIN = 0;
/* HH_FIXME[4110] */
const resource STDOUT = 0;
/* HH_FIXME[4110] */
const resource STDERR = 0;

const PATHINFO_DIRNAME = 0;
const PATHINFO_BASENAME = 0;
const PATHINFO_EXTENSION = 0;
const PATHINFO_FILENAME = 0;
const DIRECTORY_SEPARATOR = "/";
const PATH_SEPARATOR = ":";

const UPLOAD_ERR_OK = 0;
const UPLOAD_ERR_INI_SIZE = 1;
const UPLOAD_ERR_FORM_SIZE = 2;
const UPLOAD_ERR_PARTIAL = 3;
const UPLOAD_ERR_NO_FILE = 4;
const UPLOAD_ERR_NO_TMP_DIR = 6;
const UPLOAD_ERR_CANT_WRITE = 7;
const UPLOAD_ERR_EXTENSION = 8;

const FILE_APPEND = 0;
const FILE_USE_INCLUDE_PATH = 0;
const FILE_IGNORE_NEW_LINES = 0;
const FILE_NO_DEFAULT_CONTEXT = 0;
const FILE_SKIP_EMPTY_LINES = 0;

const LOCK_EX = 2;
const LOCK_SH = 1;
const LOCK_UN = 3;
const LOCK_NB = 4;

const SEEK_SET = 0;
const SEEK_CUR = 0;
const SEEK_END = 0;

const INI_SCANNER_NORMAL = 0;
const INT_SCANNER_RAW = 0;

const GLOB_ERR     = 1;
const GLOB_MARK    = 2;
const GLOB_NOSORT  = 4;
const GLOB_NOCHECK = 16;
const GLOB_ESCAPE  = 64;
const GLOB_BRACE   = 1024;
const GLOB_ONLYDIR = 8192;

<<__PHPStdLib>>
function fopen(string $filename, string $mode, bool $use_include_path = false, $context = null);
<<__PHPStdLib>>
function popen(string $command, string $mode);
<<__PHPStdLib>>
function fclose(resource $handle);
<<__PHPStdLib>>
function pclose($handle);
<<__PHPStdLib>>
function fseek(resource $handle, int $offset, int $whence = SEEK_SET);
<<__PHPStdLib>>
function rewind(resource $handle);
<<__PHPStdLib>>
function ftell(resource $handle);
<<__PHPStdLib>>
function feof(resource $handle);
<<__PHPStdLib>>
function fstat(resource $handle);
<<__PHPStdLib>>
function fread(resource $handle, int $length);
<<__PHPStdLib>>
function fgetc(resource $handle);
<<__PHPStdLib>>
function fgets(resource $handle, int $length = 0);
<<__PHPStdLib>>
function fgetss(resource $handle, int $length = 0, string $allowable_tags = "");
function fscanf(resource $handle, string $format);
<<__PHPStdLib>>
function fpassthru(resource $handle);
<<__PHPStdLib>>
function fwrite(resource $handle, string $data, int $length = 0);
<<__PHPStdLib>>
function fputs(resource $handle, string $data, int $length = 0);
<<__PHPStdLib>>
function fprintf(resource $handle, string $format, ...$args);
<<__PHPStdLib>>
function vfprintf($handle, $format, $args);
<<__PHPStdLib>>
function fflush(resource $handle);
<<__PHPStdLib>>
function ftruncate(resource $handle, int $size);
<<__PHPStdLib>>
function flock(resource $handle, int $operation, inout $wouldblock);
<<__PHPStdLib>>
function fputcsv(resource $handle, $fields, string $delimiter = ",", string $enclosure = "\"", string $escape_char = "\\");
<<__PHPStdLib>>
function fgetcsv(resource $handle, int $length = 0, string $delimiter = ",", string $enclosure = "\"", string $escape_char = "\\");
<<__PHPStdLib>>
function file_get_contents(string $filename, bool $use_include_path = false, $context = null, int $offset = 0, int $maxlen = 0);
<<__PHPStdLib>>
function file_put_contents(string $filename, $data, int $flags = 0, $context = null);
<<__PHPStdLib>>
function file(string $filename, int $flags = 0, $context = null);
<<__PHPStdLib>>
function readfile(string $filename, bool $use_include_path = false, $context = null);
<<__PHPStdLib>>
function move_uploaded_file(string $filename, string $destination);
<<__PHPStdLib>>
function parse_ini_file(string $filename, bool $process_sections = false, int $scanner_mode = INI_SCANNER_NORMAL);
<<__PHPStdLib>>
function parse_ini_string(string $ini, bool $process_sections = false, int $scanner_mode = INI_SCANNER_NORMAL);
<<__PHPStdLib>>
function md5_file(string $filename, bool $raw_output = false);
<<__PHPStdLib>>
function sha1_file(string $filename, bool $raw_output = false);
<<__PHPStdLib>>
function chmod(string $filename, int $mode);
<<__PHPStdLib>>
function chown(string $filename, $user);
<<__PHPStdLib>>
function lchown(string $filename, $user);
<<__PHPStdLib>>
function chgrp(string $filename, $group);
<<__PHPStdLib>>
function lchgrp(string $filename, $group);
<<__PHPStdLib>>
function touch(string $filename, int $mtime = 0, int $atime = 0);
<<__PHPStdLib>>
function copy(string $source, string $dest, $context = null);
<<__PHPStdLib>>
function rename(string $oldname, string $newname, $context = null);
<<__PHPStdLib>>
function umask($mask = null);
<<__PHPStdLib>>
function unlink(string $filename, $context = null);
<<__PHPStdLib>>
function link(string $target, string $link);
<<__PHPStdLib>>
function symlink(string $target, string $link);
<<__PHPStdLib, __Rx>>
function basename(string $path, string $suffix = "");
<<__PHPStdLib>>
function fnmatch(string $pattern, string $filename, int $flags = 0);
<<__PHPStdLib>>
function glob(string $pattern, int $flags = 0);
<<__PHPStdLib>>
function tempnam(string $dir, string $prefix);
<<__PHPStdLib>>
function tmpfile();
<<__PHPStdLib>>
function fileperms(string $filename);
<<__PHPStdLib>>
function fileinode(string $filename);
<<__PHPStdLib>>
function filesize(string $filename);
<<__PHPStdLib>>
function fileowner(string $filename);
<<__PHPStdLib>>
function filegroup(string $filename);
<<__PHPStdLib>>
function fileatime(string $filename);
<<__PHPStdLib>>
function filemtime(string $filename);
<<__PHPStdLib>>
function filectime(string $filename);
<<__PHPStdLib>>
function filetype(string $filename);
<<__PHPStdLib>>
function linkinfo(string $filename);
<<__PHPStdLib>>
function is_writable(string $filename);
<<__PHPStdLib>>
function is_writeable(string $filename);
<<__PHPStdLib>>
function is_readable(string $filename);
<<__PHPStdLib>>
function is_executable(string $filename);
<<__PHPStdLib>>
function is_file(string $filename);
<<__PHPStdLib>>
function is_dir(string $filename);
<<__PHPStdLib>>
function is_link(string $filename);
<<__PHPStdLib>>
function is_uploaded_file(string $filename);
<<__PHPStdLib>>
function file_exists(string $filename);
<<__PHPStdLib>>
function stat(string $filename);
<<__PHPStdLib>>
function lstat(string $filename);
<<__PHPStdLib>>
function clearstatcache();
<<__PHPStdLib>>
function readlink(string $path);
<<__PHPStdLib>>
function realpath(string $path);
<<__PHPStdLib>>
function pathinfo(string $path, int $opt = 15);
<<__PHPStdLib>>
function disk_free_space(string $directory);
<<__PHPStdLib>>
function diskfreespace(string $directory);
<<__PHPStdLib>>
function disk_total_space(string $directory);
<<__PHPStdLib>>
function mkdir(string $pathname, int $mode = 0777, bool $recursive = false, $context = null);
<<__PHPStdLib>>
function rmdir(string $dirname, $context = null);
<<__PHPStdLib, __Rx>>
function dirname(string $path);
<<__PHPStdLib>>
function getcwd();
<<__PHPStdLib>>
function chdir(string $directory);
<<__PHPStdLib>>
function chroot(string $directory);
<<__PHPStdLib>>
function dir(string $directory);
<<__PHPStdLib>>
function opendir(string $path, $context = null);
<<__PHPStdLib>>
function readdir($dir_handle);
<<__PHPStdLib>>
function rewinddir($dir_handle);
<<__PHPStdLib>>
function scandir(string $directory, bool $descending = false, $context = null);
<<__PHPStdLib>>
function closedir($dir_handle);
