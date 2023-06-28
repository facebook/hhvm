<?hh

<<__NEVER_INLINE>>
function pure_function($a) :mixed{
  return 7;
}

<<__NEVER_INLINE>>
function pure_function_2($a, $b) :mixed{
  return pure_function($a) + pure_function($b);
}

function pure_function_no_profile() :mixed{
  return null;
}

function test_exception() :mixed{
  throw new Exception('test');
}

function profiler(...$args) :mixed{
  // Calling functions from within the profiler function should note
  // use the profile (it would cause an infinite loop)
  pure_function_no_profile();
  var_dump($args);
}

function gen() :AsyncGenerator<mixed,mixed,void>{
  yield 1;
  yield 2;
}

function main() :mixed{
  pure_function_2(1, 2);
  fb_setprofile(profiler<>);
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
<<__EntryPoint>>
function entrypoint_Setprofile(): void {
  main();
}
