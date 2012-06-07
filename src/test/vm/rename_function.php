<?php

function one() {
  echo "one\n";
}

function three() {
  echo "three\n";
}

one();
var_dump(fb_rename_function("one", "two"));
two();
var_dump(fb_rename_function("three", "one"));
one();

// Try it with a builtin, too.

function my_microtime(bool $foob = false) {
  static $x = 0.0;
  echo "ca\$h m0n3y\n";
  $x += 1.0;
  if (false) {
    return $x;
  }
  return (string)$x;
}

var_dump(fb_rename_function('microtime', '__dont_call_microtime'));
var_dump(fb_rename_function('my_microtime', 'microtime'));
echo microtime(true) . "\n";

function my_foo() {}

function bar() {
  $orig = "foo";
  $new = "my_$orig";
  var_dump(fb_rename_function($new, "foo"));
}

bar();
