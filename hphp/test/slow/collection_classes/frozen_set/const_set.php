<?hh

// Check that FrozenSet is a ConstSet.

function foo(ConstSet $s) {
  foreach ($s as $e) {
    var_dump($e);
  }
}

function main() {
  foo(new FrozenSet(Vector {1, 2, 3}));
  foo(Set {4, 5, 6});
}

main();
