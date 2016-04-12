<?php

class Foo {
  public function __destruct() { echo "heh\n"; }
}

function pure_function_many_locals($a) {
  $x = new Foo;
  $xx = new Foo;
  $xxx = new Foo;
  $xxxx = new Foo;
  $xxxxx = new Foo;
  $xxxxxx = new Foo;
  $xxxxxxx = new Foo;
  $xxxxxxxx = new Foo;
  $xxxxxxxxx = new Foo;

  return 7;
}

function pure_function_2($a, $b) {
  return pure_function_many_locals($a) + pure_function_many_locals($b);
}

function pure_function_no_profile() {
  echo "yep\n";
}

function profiler() {
  // Calling functions from within the profiler function should note
  // use the profile (it would cause an infinite loop)
  pure_function_no_profile();
  var_dump(func_get_args());
}

function main() {
  pure_function_2(1, 2);
  fb_setprofile('profiler');
  pure_function_2(3, 4);
  fb_setprofile(null);
  pure_function_2(5, 6);
}
main();
