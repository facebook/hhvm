<?hh

// Test iterating over a FixedVector with a "foreach".

function main() {

  $fv = FixedVector {1, 2, 3};

  foreach ($fv as $e) {
    var_dump($e);
  }

}
main();
