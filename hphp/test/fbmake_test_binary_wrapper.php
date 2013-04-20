#!/bin/env php
<?php

chdir(__DIR__.'/../../');
include_once __DIR__.'/fbmake_test_lib.php';

$cmd = "FBMAKE_BIN_ROOT=$argv[1] " .
       "./hphp/tools/run_test_binary.sh '$argv[2]' '$argv[3]' '$argv[4]' ".
       "2>/dev/null";

loop_tests($cmd, function ($line) {
  if (preg_match('/^(Test[a-zA-Z]*)\.\.\.\.\.\.$/', $line, $m)) {
    start($m[1]);
  } else if (preg_match('/^Test[a-zA-Z]* (OK|\#\#\#\#\#\>\>\> FAILED)/',
                        $line,
                        $m)) {
    finish($m[1] == 'OK' ? 'passed' : 'failed');
  }
});
