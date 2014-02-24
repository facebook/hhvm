<?hh

// Test serialization/deserialization of FrozenSets.

function main() {
  // unserialize() o serialize() == identity function
  $fv = FrozenSet {1, 2, 3};
  var_dump(unserialize(serialize($fv)) == $fv);

  // serialization of FS literals
  var_dump(unserialize(serialize(FrozenSet {1})) == FrozenSet {1});

  // handle empty FS
  var_dump((unserialize(serialize(FrozenSet {})))->count());

  // FS with strings
  var_dump(unserialize(serialize(FrozenSet {"hello", "world"})) ==
    FrozenSet {"hello", "world"});

  // Check the actual serialized output.
  var_dump(serialize(FrozenSet {1, 2}));
}


main();
