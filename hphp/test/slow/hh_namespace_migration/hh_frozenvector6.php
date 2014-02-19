<?hh

// Test that FixedVector can be accessed through its fully-qualified name.

function main() {
  $s = HH\FixedVector { 1, 2, 3 };
  $s2 = \HH\FixedVector { 4, 5 };
  var_dump($s->count());
  var_dump($s2->count());
}

main();
