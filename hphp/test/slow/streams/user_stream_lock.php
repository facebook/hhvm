<?php

class testStreamWrapper {
  function stream_lock($operation) {
    var_dump($operation);
    return true;
  }
  function stream_open($path, $mode, $options, $opened_path) {
    return true;
  }
}

stream_wrapper_register('test', 'TestStreamWrapper');

$fp = fopen('test://foo', 'w+');
flock($fp, LOCK_SH);
flock($fp, LOCK_EX);
flock($fp, LOCK_UN);
flock($fp, LOCK_SH | LOCK_NB);
flock($fp, LOCK_EX | LOCK_NB);
