<?hh

// Test that we can handle unserialization of Pairs with
// and without namespaces.

function main() {
  $p = Pair {1, 2};

  // unserialize o serialize == id
  var_dump(unserialize(serialize($p)) == $p);

  // Namespaced pair.
  $nsp = "V:7:\"HH\\Pair\":2:{i:1;i:2;}";
  var_dump(unserialize($nsp) == $p);

  // Nested namespaced pairs.
  $nested = "V:7:\"HH\\Pair\":2:{V:7:\"HH\\Pair\":2:{i:1;i:2;}" .
            "V:7:\"HH\\Pair\":2:{i:3;i:4;}}";
  var_dump(unserialize($nested) == Pair {Pair {1, 2}, Pair {3, 4}});

  // Unserialization is case-insensitive.
  $s1 = "V:7:\"hh\\pair\":2:{i:1;i:2;}";
  $s2 = "V:7:\"HH\\PaiR\":2:{i:1;i:2;}";
  $s3 = "V:4:\"pair\":2:{i:1;i:2;}";
  $s4 = "V:4:\"Pair\":2:{i:1;i:2;}";

  var_dump(unserialize($s1) == $p);
  var_dump(unserialize($s2) == $p);
  var_dump(unserialize($s3) == $p);
  var_dump(unserialize($s4) == $p);
}

main();
