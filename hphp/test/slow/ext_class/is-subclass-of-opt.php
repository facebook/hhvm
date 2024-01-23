<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class B {}
class A extends B {}

function get_sub_b() :mixed{
  return extension_loaded('pdo') ? (new B()) : (new A());
}

function main() :mixed{
  // Shouldn't be optimized, don't support the first arg being a string yet.
  $x0 = is_subclass_of('A', 'B');
  $x1 = is_subclass_of('A', 'B', false);
  $x2 = is_subclass_of('B', 'A');
  $x3 = is_subclass_of('B', 'A', false);

  // Should be optimized.
  $x4 = is_subclass_of((new A()), 'B');
  $x5 = is_subclass_of((new A()), 'B', false);
  $x6 = is_subclass_of((new B()), 'A');
  $x7 = is_subclass_of((new B()), 'A', false);

  // Shouldn't be optimized, types don't match.
  try { is_subclass_of('A', (new B())); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { is_subclass_of('A', (new B()), false); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  $x10 = is_subclass_of(get_sub_b(), 'B');
  $x11 = is_subclass_of(get_sub_b(), 'A');

  return vec[$x0, $x1, $x2, $x3, $x4, $x5, $x6, $x7, $x10, $x11];
}


<<__EntryPoint>>
function main_is_subclass_of_opt(): void {
  set_error_handler(($errno, $errstr, ...) ==> {
    throw new Exception($errstr);
  });

  main();
  main();
  main();

  var_dump(main());
}
