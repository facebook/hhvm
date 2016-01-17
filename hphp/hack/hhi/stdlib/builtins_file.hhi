<?hh // decl /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
const STDIN = 0;
const STDOUT = 0;
const STDERR = 0;

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

function fopen($filename, $mode, $use_include_path = false, $context = null);
function popen($command, $mode);
function fclose($handle);
function pclose($handle);
function fseek($handle, $offset, $whence = SEEK_SET);
function rewind($handle);
function ftell($handle);
function feof($handle);
function fstat($handle);
function fread($handle, $length);
function fgetc($handle);
function fgets($handle, $length = 0);
function fgetss($handle, $length = 0, $allowable_tags = null);
function fscanf($handle, $format);
function fpassthru($handle);
function fwrite($handle, $data, $length = 0);
function fputs($handle, $data, $length = 0);
function fprintf($handle, $format, ...);
function vfprintf($handle, $format, $args);
function fflush($handle);
function ftruncate($handle, $size);
function flock($handle, $operation, &$wouldblock = null);
function fputcsv($handle, $fields, $delimiter = ",", $enclosure = "\"");
function fgetcsv($handle, $length = 0, $delimiter = ",", $enclosure = "\"");
function file_get_contents($filename, $use_include_path = false, $context = null, $offset = 0, $maxlen = 0);
function file_put_contents($filename, $data, $flags = 0, $context = null);
function file($filename, $flags = 0, $context = null);
function readfile($filename, $use_include_path = false, $context = null);
function move_uploaded_file($filename, $destination);
function parse_ini_file($filename, $process_sections = false, $scanner_mode = INI_SCANNER_NORMAL);
function parse_ini_string($ini, $process_sections = false, $scanner_mode = INI_SCANNER_NORMAL);
function md5_file($filename, $raw_output = false);
function sha1_file($filename, $raw_output = false);
function chmod($filename, $mode);
function chown($filename, $user);
function lchown($filename, $user);
function chgrp($filename, $group);
function lchgrp($filename, $group);
function touch($filename, $mtime = 0, $atime = 0);
function copy($source, $dest, $context = null);
function rename($oldname, $newname, $context = null);
function umask($mask = null);
function unlink($filename, $context = null);
function link($target, $link);
function symlink($target, $link);
function basename($path, $suffix = null);
function fnmatch($pattern, $filename, $flags = 0);
function glob($pattern, $flags = 0);
function tempnam($dir, $prefix);
function tmpfile();
function fileperms($filename);
function fileinode($filename);
function filesize(string $filename);
function fileowner($filename);
function filegroup($filename);
function fileatime($filename);
function filemtime($filename);
function filectime($filename);
function filetype($filename);
function linkinfo($filename);
function is_writable($filename);
function is_writeable($filename);
function is_readable($filename);
function is_executable($filename);
function is_file($filename);
function is_dir($filename);
function is_link($filename);
function is_uploaded_file($filename);
function file_exists($filename);
function stat($filename);
function lstat($filename);
function clearstatcache();
function readlink($path);
function realpath($path);
function pathinfo($path, $opt = 15);
function disk_free_space($directory);
function diskfreespace($directory);
function disk_total_space($directory);
function mkdir($pathname, $mode = 0777, $recursive = false, $context = null);
function rmdir($dirname, $context = null);
function dirname($path);
function getcwd();
function chdir($directory);
function chroot($directory);
function dir($directory);
function opendir($path, $context = null);
function readdir($dir_handle);
function rewinddir($dir_handle);
function scandir($directory, $descending = false, $context = null);
function closedir($dir_handle);
