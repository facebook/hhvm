<?hh

// Test serialization/deserialization of FrozenVectors.

function main() {
  // unserialize() o serialize() == identity function
  $fv = FrozenVector {1, 2, 3};
  var_dump(unserialize(serialize($fv)) == $fv);

  // serialization of FV literals
  var_dump(unserialize(serialize(FrozenVector {1})) == FrozenVector {1});

  // handle empty FV
  var_dump((unserialize(serialize(FrozenVector {})))->count());

  // we can now properly var_dump() FVs
  var_dump($fv);

  // nested FV's
  $a = FrozenVector {1, 2};
  $b = FrozenVector {3};
  $expected = FrozenVector {$a, $b};
  var_dump(unserialize(serialize($expected)) == $expected);
}


main();
