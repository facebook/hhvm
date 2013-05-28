<?php

class MagicStream {
  public function __call($fname, $args) {
    echo "Method: $fname\n";
    if ($fname == 'stream_open') $args[2] &= ~4;
    var_dump($args);
    return true;
  }
}
stream_wrapper_register('magic', 'MagicStream');
$fp = fopen('magic://stream-via-call', 'r');
fclose($fp);
