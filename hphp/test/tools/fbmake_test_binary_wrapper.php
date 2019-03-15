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
ToolsFbmakeTestBinaryWrapperPhp::$results = array();
ToolsFbmakeTestBinaryWrapperPhp::$current = '';

function finish($status) {



  say(array('op' => 'test_done',
            'test' => ToolsFbmakeTestBinaryWrapperPhp::$current,
            'details' => '',
            'status' => $status));
  array_push(ToolsFbmakeTestBinaryWrapperPhp::$results, array('name'   => ToolsFbmakeTestBinaryWrapperPhp::$current,
                             'status' => $status));
  ToolsFbmakeTestBinaryWrapperPhp::$current = '';
}

function start($test) {


  ToolsFbmakeTestBinaryWrapperPhp::$current = $test;
  say(array('op'    => 'start',
            'test'  => ToolsFbmakeTestBinaryWrapperPhp::$current));
}

function test_is_running() {
  return $GLOBALS['current'] != '';
}

function loop_tests($cmd, $line_func) {


  $ftest = popen($cmd, 'r');
  if (!$ftest) {
    echo "Couldn't run test script\n";
    exit(1);
  }
  while (!feof($ftest)) {
    $line = fgets($ftest);
    $line_func($line);
  }
  if (!fclose($ftest)) {

    if (ToolsFbmakeTestBinaryWrapperPhp::$current !== '') {
      finish('failed');
    }
    start('test-binary');
    finish('failed');
    return;
  }

  say(array('op'      => 'all_done',
            'results' => ToolsFbmakeTestBinaryWrapperPhp::$results));
}


chdir(__DIR__.'/../../../');
$cmd = "./hphp/tools/run_test_binary.sh " .
       "'$argv[1]' '$argv[2]' '$argv[3]' '$argv[4]' ".
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

abstract final class ToolsFbmakeTestBinaryWrapperPhp {
  public static $results;
  public static $current;
}
