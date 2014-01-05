<?hh

// Test that Set can be accessed through its fully-qualified name.

function main() {
  $s = HH\Set { 1, 2, 3 };
  $s2 = \HH\Set { 4, 5 };
  var_dump($s->count());
  var_dump($s2->count());
}

main();
