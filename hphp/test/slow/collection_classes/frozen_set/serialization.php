<?hh

// Test serialization/deserialization of FixedSets.

function main() {
  // unserialize() o serialize() == identity function
  $fv = FixedSet {1, 2, 3};
  var_dump(unserialize(serialize($fv)) == $fv);

  // serialization of FS literals
  var_dump(unserialize(serialize(FixedSet {1})) == FixedSet {1});

  // handle empty FS
  var_dump((unserialize(serialize(FixedSet {})))->count());

  // FS with strings
  var_dump(unserialize(serialize(FixedSet {"hello", "world"})) ==
    FixedSet {"hello", "world"});

  // Check the actual serialized output.
  var_dump(serialize(FixedSet {1, 2}));
}


main();
