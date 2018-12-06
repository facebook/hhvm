<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const resource STDIN = /* UNSAFE_EXPR */ 0;
const resource STDOUT = /* UNSAFE_EXPR */ 0;
const resource STDERR = /* UNSAFE_EXPR */ 0;

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
function fopen($filename, $mode, $use_include_path = false, $context = null);
<<__PHPStdLib>>
function popen($command, $mode);
<<__PHPStdLib>>
function fclose($handle);
<<__PHPStdLib>>
function pclose($handle);
<<__PHPStdLib>>
function fseek($handle, $offset, $whence = SEEK_SET);
<<__PHPStdLib>>
function rewind($handle);
<<__PHPStdLib>>
function ftell($handle);
<<__PHPStdLib>>
function feof($handle);
<<__PHPStdLib>>
function fstat($handle);
<<__PHPStdLib>>
function fread($handle, $length);
<<__PHPStdLib>>
function fgetc($handle);
<<__PHPStdLib>>
function fgets($handle, $length = 0);
<<__PHPStdLib>>
function fgetss($handle, $length = 0, $allowable_tags = null);
function fscanf($handle, $format);
<<__PHPStdLib>>
function fpassthru($handle);
<<__PHPStdLib>>
function fwrite($handle, $data, $length = 0);
<<__PHPStdLib>>
function fputs($handle, $data, $length = 0);
<<__PHPStdLib>>
function fprintf($handle, $format, ...);
<<__PHPStdLib>>
function vfprintf($handle, $format, $args);
<<__PHPStdLib>>
function fflush($handle);
<<__PHPStdLib>>
function ftruncate($handle, $size);
<<__PHPStdLib>>
function flock($handle, $operation, &$wouldblock = null);
<<__PHPStdLib>>
function fputcsv($handle, $fields, $delimiter = ",", $enclosure = "\"", $escape_char = "\\");
<<__PHPStdLib>>
function fgetcsv($handle, $length = 0, $delimiter = ",", $enclosure = "\"", $escape_char = "\\");
<<__PHPStdLib>>
function file_get_contents($filename, $use_include_path = false, $context = null, $offset = 0, $maxlen = 0);
<<__PHPStdLib>>
function file_put_contents($filename, $data, $flags = 0, $context = null);
<<__PHPStdLib>>
function file($filename, $flags = 0, $context = null);
<<__PHPStdLib>>
function readfile($filename, $use_include_path = false, $context = null);
<<__PHPStdLib>>
function move_uploaded_file($filename, $destination);
<<__PHPStdLib>>
function parse_ini_file($filename, $process_sections = false, $scanner_mode = INI_SCANNER_NORMAL);
<<__PHPStdLib>>
function parse_ini_string($ini, $process_sections = false, $scanner_mode = INI_SCANNER_NORMAL);
<<__PHPStdLib>>
function md5_file($filename, $raw_output = false);
<<__PHPStdLib>>
function sha1_file($filename, $raw_output = false);
<<__PHPStdLib>>
function chmod($filename, $mode);
<<__PHPStdLib>>
function chown($filename, $user);
<<__PHPStdLib>>
function lchown($filename, $user);
<<__PHPStdLib>>
function chgrp($filename, $group);
<<__PHPStdLib>>
function lchgrp($filename, $group);
<<__PHPStdLib>>
function touch($filename, $mtime = 0, $atime = 0);
<<__PHPStdLib>>
function copy($source, $dest, $context = null);
<<__PHPStdLib>>
function rename($oldname, $newname, $context = null);
<<__PHPStdLib>>
function umask($mask = null);
<<__PHPStdLib>>
function unlink($filename, $context = null);
<<__PHPStdLib>>
function link($target, $link);
<<__PHPStdLib>>
function symlink($target, $link);
<<__PHPStdLib, __Rx>>
function basename($path, $suffix = null);
<<__PHPStdLib>>
function fnmatch($pattern, $filename, $flags = 0);
<<__PHPStdLib>>
function glob($pattern, $flags = 0);
<<__PHPStdLib>>
function tempnam($dir, $prefix);
<<__PHPStdLib>>
function tmpfile();
<<__PHPStdLib>>
function fileperms($filename);
<<__PHPStdLib>>
function fileinode($filename);
<<__PHPStdLib>>
function filesize(?Stringish $filename);
<<__PHPStdLib>>
function fileowner($filename);
<<__PHPStdLib>>
function filegroup($filename);
<<__PHPStdLib>>
function fileatime($filename);
<<__PHPStdLib>>
function filemtime($filename);
<<__PHPStdLib>>
function filectime($filename);
<<__PHPStdLib>>
function filetype($filename);
<<__PHPStdLib>>
function linkinfo($filename);
<<__PHPStdLib>>
function is_writable($filename);
<<__PHPStdLib>>
function is_writeable($filename);
<<__PHPStdLib>>
function is_readable($filename);
<<__PHPStdLib>>
function is_executable($filename);
<<__PHPStdLib>>
function is_file($filename);
<<__PHPStdLib>>
function is_dir($filename);
<<__PHPStdLib>>
function is_link($filename);
<<__PHPStdLib>>
function is_uploaded_file($filename);
<<__PHPStdLib>>
function file_exists($filename);
<<__PHPStdLib>>
function stat($filename);
<<__PHPStdLib>>
function lstat($filename);
<<__PHPStdLib>>
function clearstatcache();
<<__PHPStdLib>>
function readlink($path);
<<__PHPStdLib>>
function realpath($path);
<<__PHPStdLib>>
function pathinfo($path, $opt = 15);
<<__PHPStdLib>>
function disk_free_space($directory);
<<__PHPStdLib>>
function diskfreespace($directory);
<<__PHPStdLib>>
function disk_total_space($directory);
<<__PHPStdLib>>
function mkdir($pathname, $mode = 0777, $recursive = false, $context = null);
<<__PHPStdLib>>
function rmdir($dirname, $context = null);
<<__PHPStdLib, __Rx>>
function dirname($path);
<<__PHPStdLib>>
function getcwd();
<<__PHPStdLib>>
function chdir($directory);
<<__PHPStdLib>>
function chroot($directory);
<<__PHPStdLib>>
function dir($directory);
<<__PHPStdLib>>
function opendir($path, $context = null);
<<__PHPStdLib>>
function readdir($dir_handle);
<<__PHPStdLib>>
function rewinddir($dir_handle);
<<__PHPStdLib>>
function scandir($directory, $descending = false, $context = null);
<<__PHPStdLib>>
function closedir($dir_handle);
