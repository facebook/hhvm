<?php

class A {
  public function dir_closedir() {
    return true;
  }

  public function dir_opendir($path, $options) {
    return true;
  }

  public function dir_readdir() {
    return true;
  }

  public function dir_rewinddir() {
    return true;
  }

  public function mkdir($path, $mode, $options) {
    return true;
  }

  public function rename($path_from, $path_to) {
    return true;
  }

  public function rmdir($path, $options) {
    return true;
  }

  public function stream_cast($cast_as) {
    return false;
  }

  public function stream_close() {
    return true;
  }

  public function stream_eof() {
    return false;
  }

  public function stream_flush() {
    return true;
  }

  public function stream_lock($operation) {
    return true;
  }

  public function stream_metadata($path, $option, $value) {
    var_dump(__FUNCTION__);
    var_dump(func_get_args());
    return true;
  }

  public function stream_open($path, $mode, $options, &$opened_path) {
    return true;
  }

  public function stream_read($count) {
    return true;
  }

  public function stream_seek($offset, $whence = SEEK_SET) {
    return true;
  }

  public function stream_set_option($option, $arg1, $arg2) {
    return true;
  }

  public function stream_stat() {
    return true;
  }

  public function stream_tell() {
    return true;
  }

  public function stream_truncate($new_size) {
    return true;
  }

  public function stream_write($data) {
    return true;
  }

  public function unlink($path) {
    return true;
  }

  public function url_stat($path, $flags) {
    return true;
  }

}

stream_wrapper_register("abc", "A") or die("Failed to register protocol");


touch('abc://a');
touch('abc://b', 15);
touch('abc://c', 15, 25);

chmod('abc://d', 0123);
chmod('abc://e', 0345);

chown('abc://f', 0);
chown('abc://g', 10);
chown('abc://h', 'root');

lchown('abc://i', 0);
lchown('abc://j', 10);
lchown('abc://k', 'root');

chgrp('abc://l', 0);
chgrp('abc://m', 10);
chgrp('abc://n', 'root');

lchgrp('abc://o', 0);
lchgrp('abc://p', 10);
lchgrp('abc://q', 'root');

// explicit errors

chown('abc://r', []);
lchown('abc://s', []);
chgrp('abc://t', []);
lchgrp('abc://u', []);
