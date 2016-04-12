<?php

function pure_function($a) {
  return 7;
}

function pure_function_2($a, $b) {
  return pure_function($a) + pure_function($b);
}

function pure_function_no_profile() {
  return null;
}

function test_exception() {
  throw new Exception('test');
}

function profiler() {
  // Calling functions from within the profiler function should note
  // use the profile (it would cause an infinite loop)
  pure_function_no_profile();
  var_dump(func_get_args());
}

function gen() {
  yield 1;
  yield 2;
}

function main() {
  pure_function_2(1, 2);
  fb_setprofile('profiler');
  pure_function_2(3, 4);
  srand(0xdeadbeef);
  try {
    test_exception();
  } catch (Exception $e) {
    //do nothing.
  }
  foreach (gen() as $x) {}
  fb_setprofile(null);
  pure_function_2(5, 6);
}
main();
