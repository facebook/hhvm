<?hh

// Test serialization/deserialization of FixedVectors.

function main() {
  // unserialize() o serialize() == identity function
  $fv = FixedVector {1, 2, 3};
  var_dump(unserialize(serialize($fv)) == $fv);

  // serialization of FV literals
  var_dump(unserialize(serialize(FixedVector {1})) == FixedVector {1});

  // handle empty FV
  var_dump((unserialize(serialize(FixedVector {})))->count());

  // we can now properly var_dump() FVs
  var_dump($fv);

  // nested FV's
  $a = FixedVector {1, 2};
  $b = FixedVector {3};
  $expected = FixedVector {$a, $b};
  var_dump(unserialize(serialize($expected)) == $expected);
}


main();
