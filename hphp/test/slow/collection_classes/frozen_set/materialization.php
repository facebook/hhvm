<?hh

// Test materialization *to* FixedSet.

function main() {
  $ref = new FixedSet(Vector {1, 2, 3});

  var_dump((Vector {1, 2, 3})->toFixedSet() == $ref);
  var_dump((FixedVector {1, 2, 3})->toFixedSet() == $ref);
  var_dump((Set {1, 2, 3})->toFixedSet() == $ref);
  var_dump((FixedSet {1, 2, 3})->toFixedSet() == $ref);
  var_dump((Map {0 => 1, 1 => 2, 2 => 3})->toFixedSet() == $ref);
  var_dump((StableMap {0 => 1, 1 => 2, 2 => 3})->toFixedSet() == $ref);
  var_dump((Pair {1, 2})->toFixedSet() == FixedSet {1, 2});
}

main();
