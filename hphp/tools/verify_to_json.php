#!/bin/env php
<?php

$HPHP_HOME = $_ENV['HPHP_HOME'];
include_once $HPHP_HOME.'/hphp/test/fbmake_test_lib.php';

//////////////////////////////////////////////////////////////////////

if (count($argv) != 5) {
  echo "usage: $argv[0] test-script interp|jit|hhir TEST_PATH FBMAKE_BIN_ROOT\n";
  exit(1);
}

$cmd = "VQ=$argv[2] TEST_PATH=$argv[3] FBMAKE_BIN_ROOT=$HPHP_HOME/$argv[4] " .
       "$HPHP_HOME/hphp/tools/$argv[1]";

loop_tests($cmd, function ($line) {
  if (preg_match('/^(test[^\s]*).*/', $line, &$m)) {
    start($m[1]);
    return;
  }
  if (!test_is_running()) return;
  if (preg_match('/^\s*passed.*/', $line)) {
    finish('passed');
  } else if (preg_match('/^[\s\*]*FAILED.*/', $line)) {
    finish('failed');
  }
});
