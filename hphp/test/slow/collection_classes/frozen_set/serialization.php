<?hh

// Test serialization/deserialization of ImmSets.

function main() {
  // unserialize() o serialize() == identity function
  $fv = ImmSet {1, 2, 3};
  var_dump(unserialize(serialize($fv)) == $fv);

  // serialization of FS literals
  var_dump(unserialize(serialize(ImmSet {1})) == ImmSet {1});

  // handle empty FS
  var_dump((unserialize(serialize(ImmSet {})))->count());

  // FS with strings
  var_dump(unserialize(serialize(ImmSet {"hello", "world"})) ==
    ImmSet {"hello", "world"});

  // Check the actual serialized output.
  var_dump(serialize(ImmSet {1, 2}));
}


main();
