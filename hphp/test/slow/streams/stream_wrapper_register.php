<?php

class A {
  function __construct() {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return true;
  }

  function __destruct() {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return true;
  }

  public function dir_closedir() {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return true;
  }

  public function dir_opendir($path, $options) {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return true;
  }

  public function dir_readdir() {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return true;
  }

  public function dir_rewinddir() {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return true;
  }

  public function mkdir($path, $mode, $options) {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return true;
  }

  public function rename($path_from, $path_to) {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return true;
  }

  public function rmdir($path, $options) {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return true;
  }

  public function stream_cast($cast_as) {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return false;
  }

  public function stream_close() {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return true;
  }

  public function stream_eof() {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return false;
  }

  public function stream_flush() {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return true;
  }

  public function stream_lock($operation) {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return true;
  }

  public function stream_metadata($path, $option, $value) {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return true;
  }

  public function stream_open($path, $mode, $options, &$opened_path) {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return true;
  }

  public function stream_read($count) {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return true;
  }

  public function stream_seek($offset, $whence = SEEK_SET) {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return true;
  }

  public function stream_set_option($option, $arg1, $arg2) {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return true;
  }

  public function stream_stat() {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return true;
  }

  public function stream_tell() {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return true;
  }

  public function stream_truncate($new_size) {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return true;
  }

  public function stream_write($data) {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return true;
  }

  public function unlink($path) {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return true;
  }

  public function url_stat($path, $flags) {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return true;
  }

}

stream_wrapper_register("abc", "A") or die("Failed to register protocol");

$fp = fopen("abc://b", "rw");
feof($fp);
fread($fp, 1);

/** We don't have anything like stream_cast
$r = array($fp);
$w = null;
$e = null;
stream_select($r, $w, $e, 0);
*/

fflush($fp);
flock($fp, LOCK_SH);
/** No virtual method yet
stream_set_blocking($fp, 1);
*/
fseek($fp, 123);
/** No virtual method yet
fstat($fp);
*/
ftruncate($fp, 456);
fwrite($fp, 789);

chmod('abc://c', 0123);
unlink('abc://d');
is_file('abc://e');
is_link('abc://f');
touch('abc://g');
