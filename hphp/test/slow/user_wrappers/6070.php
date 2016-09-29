<?php

class TestStream {
  function stream_open($path, $mode, $options, &$openedPath) {
    return true;
  }
  function stream_read($data) { }
  function stream_write($data) { }
  function stream_tell() {
    return 42;
  }
  function stream_seek($offset, $whence) { return true; }
}

stream_wrapper_register('test', 'TestStream');

$f = fopen('test://nonempty2nd', 'r');
var_dump(ftell($f));

$f = fopen('test://nonempty2nd', 'r+');
var_dump(ftell($f));

$f = fopen('test://nonempty2nd', 'w');
var_dump(ftell($f));

$f = fopen('test://nonempty2nd', 'w+');
var_dump(ftell($f));

$f = fopen('test://nonempty2nd', 'a');
var_dump(ftell($f));

$f = fopen('test://nonempty2nd', 'a+');
var_dump(ftell($f));
