<?hh

// Test that we can handle unserialization of Sets with
// and without namespaces.

function main() {
  $set = Set {1, 2, 3};

  // unserialize o serialize == id
  var_dump(unserialize(serialize($set)) == $set);

  // Namespaced Set.
  $nsset = "V:6:\"HH\\Set\":3:{i:1;i:2;i:3;}";
  var_dump(unserialize($nsset) == $set);

  // O format
  $o1 = "O:3:\"Set\":0:{}";
  $o2 = "O:6:\"HH\\Set\":0:{}";
  var_dump(unserialize($o1) == Set {});
  var_dump(unserialize($o2) == Set {});

  // Unserialization is case-insensitive.
  $s1 = "V:6:\"hh\\set\":3:{i:1;i:2;i:3;}";
  $s2 = "V:6:\"HH\\SeT\":3:{i:1;i:2;i:3;}";
  $s3 = "V:3:\"set\":3:{i:1;i:2;i:3;}";
  $s4 = "V:3:\"Set\":3:{i:1;i:2;i:3;}";

  var_dump(unserialize($s1) == $set);
  var_dump(unserialize($s2) == $set);
  var_dump(unserialize($s3) == $set);
  var_dump(unserialize($s4) == $set);
}

main();
