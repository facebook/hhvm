#!/bin/env php
<?php
chdir(__DIR__.'/../../');

include_once 'hphp/test/fbmake_test_lib.php';

//////////////////////////////////////////////////////////////////////

// Args to this script are:
//   $argv[0] test-script test-path interp|jit|hhir maybe-dash-r
//            FBMAKE_BIN_ROOT
if (count($argv) != 6) {
  echo
"This script is not intended for direct use.  See hphp/hhvm/TARGETS.\n";
  exit(1);
}

$cmd = "FBMAKE_BIN_ROOT=$argv[5] $argv[1] $argv[2] -m $argv[3] $argv[4]";
loop_tests($cmd, function ($line) {
  if (preg_match('/^(?:hphp\/)?(test[^\s]*).*/', $line, &$m)) {
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
