<?php

function bar() { return count(func_get_args()); }
function baz() {}

function test($f, $x) {
  call_user_func_array($f, array($x));
  baz();
}

function main() {
  $x[] = 1;
  fb_intercept('bar', 'baz', 'fiz');
  for ($i = 0; $i < 10000; $i++) {
    test('bar', $x);
  }
  var_dump('done');
}

main();

