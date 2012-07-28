<?php

// XXXjit doesn't do "missing argument" warnings
error_reporting(0);

function f($a, $b) {
  $args = func_get_args();
  var_dump($args);

  if (count($args) != func_num_args()) {
    echo "num args fail: " . count($args) . " " . func_num_args() . "\n";
  }
  foreach ($args as $i => $arg) {
    if ($arg != func_get_arg($i)) {
      echo "get arg fail: $arg " . func_get_arg($i) . "\n";
    }
  }
}

f();
f(12);
f(12, 34);
f(12, 34, 56);

function defaulte($a, $b=999) {
  $args = func_get_args();
  var_dump($args);

  if (count($args) != func_num_args()) {
    echo "num args fail: " . count($args) . " " . func_num_args() . "\n";
  }
  foreach ($args as $i => $arg) {
    if ($arg != func_get_arg($i)) {
      echo "get arg fail: $arg " . func_get_arg($i) . "\n";
    }
  }
}

defaulte();
defaulte(12);
defaulte(12, 34);
defaulte(12, 34, 56);
