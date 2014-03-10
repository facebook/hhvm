<?hh

// Test materialization *to* ImmSet.

function main() {
  $ref = new ImmSet(Vector {1, 2, 3});

  var_dump((Vector {1, 2, 3})->toImmSet() == $ref);
  var_dump((ImmVector {1, 2, 3})->toImmSet() == $ref);
  var_dump((Set {1, 2, 3})->toImmSet() == $ref);
  var_dump((ImmSet {1, 2, 3})->toImmSet() == $ref);
  var_dump((Map {0 => 1, 1 => 2, 2 => 3})->toImmSet() == $ref);
  var_dump((ImmMap {0 => 1, 1 => 2, 2 => 3})->toImmSet() == $ref);
  var_dump((Pair {1, 2})->toImmSet() == ImmSet {1, 2});
}

main();
