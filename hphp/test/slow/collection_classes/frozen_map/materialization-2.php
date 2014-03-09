<?hh

// Test materialization *to* ImmMap.

function main() {
  $ref = ImmMap {0 => 1, 1 => 2, 2 => 3};

  var_dump((Vector {1, 2, 3})->toImmMap() == $ref);
  var_dump((ImmVector {1, 2, 3})->toImmMap() == $ref);
  var_dump((Map {0 => 1, 1 => 2, 2 => 3})->toImmMap() == $ref);
  var_dump((ImmMap {0 => 1, 1 => 2, 2 => 3})->toImmMap() == $ref);
  var_dump((Pair {1, 2})->toImmMap() == ImmMap {0 => 1, 1 => 2});
}

main();
