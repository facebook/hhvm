<?hh

// Test serialization/deserialization of ImmVectors.

function main() {
  // unserialize() o serialize() == identity function
  $fv = ImmVector {1, 2, 3};
  var_dump(unserialize(serialize($fv)) == $fv);

  // serialization of FV literals
  var_dump(unserialize(serialize(ImmVector {1})) == ImmVector {1});

  // handle empty FV
  var_dump((unserialize(serialize(ImmVector {})))->count());

  // we can now properly var_dump() FVs
  var_dump($fv);

  // nested FV's
  $a = ImmVector {1, 2};
  $b = ImmVector {3};
  $expected = ImmVector {$a, $b};
  var_dump(unserialize(serialize($expected)) == $expected);
}


main();
