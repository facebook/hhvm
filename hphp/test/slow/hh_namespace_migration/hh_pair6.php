<?hh

// Test that Pair can be accessed through its fully-qualified name.

function main() {
  $s = HH\Pair { 1, 2 };
  $s2 = \HH\Pair { 4, 5 };
  var_dump($s->count());
  var_dump($s2->count());
}

main();
