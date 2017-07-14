<?php
class CLWrapper {
  function stream_open($path, $mode, $options, $opened_path) {
    return false;
  }
}

class CL {
  public function load($class) {
    if (!include($class)) {
      throw new Exception('Failed loading '.$class);
    }
  }
}

stream_wrapper_register('class', 'CLWrapper');
set_error_handler(function($code, $msg, $file, $line) {
  $bt= debug_backtrace(DEBUG_BACKTRACE_IGNORE_ARGS, 2);
  echo "ERR#$code: $msg @ ", $bt[1]['function'], "\n";
});

try {
  (new CL())->load('class://non.existent.Class');
} catch (CLException $e) {
  echo $e."\n";
}
