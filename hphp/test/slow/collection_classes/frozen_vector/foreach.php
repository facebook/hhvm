<?hh

// Test iterating over a FrozenVector with a "foreach".

function main() {

  $fv = FrozenVector {1, 2, 3};

  foreach ($fv as $e) {
    var_dump($e);
  }

}
main();
