<?hh

// Test array-like iteration for ImmVector.

function main() {

  $fv = ImmVector {1, 2, 3, 4};

  for ($i = 0; $i < $fv->count(); $i++) {
    var_dump($fv[$i]);
  }

}

main();
