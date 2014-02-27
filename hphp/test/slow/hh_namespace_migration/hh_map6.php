<?hh

// Test that Map can be accessed through its fully-qualified name.

function main() {
  $m = HH\Map { 1 => 1, 2 => 2, 3 => 3};
  $m2 = \HH\Map { 4 => 4, 5 => 5 };
  var_dump($m->count());
  var_dump($m2->count());
}

main();
