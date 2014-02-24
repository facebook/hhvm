<?hh

// Test array-like iteration for FrozenVector.

function main() {

  $fv = FrozenVector {1, 2, 3, 4};

  for ($i = 0; $i < $fv->count(); $i++) {
    var_dump($fv[$i]);
  }

}

main();
