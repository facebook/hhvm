<?hh

// Test that the JIT triggers COW.

function main() {
  $v = Vector {1, 2, 3};
  $fv = $v->toFrozenVector();
  $v[0] = 10; // This is handled in the JIT, and should it trigger COW.
  var_dump($v);
  var_dump($fv);
}

main();
