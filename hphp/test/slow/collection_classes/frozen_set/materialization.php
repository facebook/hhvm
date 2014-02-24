<?hh

// Test materialization *to* FrozenSet.

function main() {
  $ref = new FrozenSet(Vector {1, 2, 3});

  var_dump((Vector {1, 2, 3})->toFrozenSet() == $ref);
  var_dump((FrozenVector {1, 2, 3})->toFrozenSet() == $ref);
  var_dump((Set {1, 2, 3})->toFrozenSet() == $ref);
  var_dump((FrozenSet {1, 2, 3})->toFrozenSet() == $ref);
  var_dump((Map {0 => 1, 1 => 2, 2 => 3})->toFrozenSet() == $ref);
  var_dump((StableMap {0 => 1, 1 => 2, 2 => 3})->toFrozenSet() == $ref);
  var_dump((Pair {1, 2})->toFrozenSet() == FrozenSet {1, 2});
}

main();
