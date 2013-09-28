#!/bin/env php
<?php

/*
 * Small utilities for wrapping tests to put output into fbmake.
 *
 * See hphp/hhvm/fbmake_test_ext_wrapper.php.
 */

// Output is in the format expected by JsonTestRunner.
function say($val) {
  fwrite(STDERR, json_encode($val, JSON_UNESCAPED_SLASHES) . "\n");
}

// Currently running test, and the results of each test.
$results = array();
$current = '';

function finish($status) {
  global $results;
  global $current;

  say(array('op' => 'test_done',
            'test' => $current,
            'details' => '',
            'status' => $status));
  array_push($results, array('name'   => $current,
                             'status' => $status));
  $current = '';
}

function start($test) {
  global $current;

  $current = $test;
  say(array('op'    => 'start',
            'test'  => $current));
}

function test_is_running() {
  return $GLOBALS['current'] != '';
}

function loop_tests($cmd, $line_func) {
  global $results;

  $ftest = popen($cmd, 'r');
  if (!$ftest) {
    echo "Couldn't run test script\n";
    exit(1);
  }
  while (!feof($ftest)) {
    $line = fgets($ftest);
    $line_func($line);
  }
  say(array('op'      => 'all_done',
            'results' => $results));
  fclose($ftest);
}


chdir(__DIR__.'/../../../');
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
