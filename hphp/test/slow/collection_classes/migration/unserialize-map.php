<?hh

// Test that we can handle unserialization of Maps with
// and without namespaces.

function main() {
  $m = Map {'a' => 1, 'b' => 2};

  // unserialize o serialize == id
  var_dump(unserialize(serialize($m)) == $m);

  // Namespaced Map.
  $nsm = "K:6:\"HH\\Map\":2:{s:1:\"a\";i:1;s:1:\"b\";i:2;}";
  var_dump(unserialize($nsm) == $m);

  // Nested namespaced Map.
  $nested = "K:6:\"HH\\Map\":2:{i:0;K:6:\"HH\\Map\":0:{}" .
            "i:1;K:6:\"HH\\Map\":1:{s:1:\"a\";i:2;}}";
  var_dump(unserialize($nested) == Map {0 => Map {}, 1 => Map {'a' => 2}});

  // O format
  $o1 = "O:3:\"Map\":0:{}";
  $o2 = "O:6:\"HH\\Map\":0:{}";
  var_dump(unserialize($o1) == Map {});
  var_dump(unserialize($o2) == Map {});

  // Unserialization is case-insensitive.
  $s1 = "K:6:\"hh\\map\":2:{s:1:\"a\";i:1;s:1:\"b\";i:2;}";
  $s2 = "K:6:\"HH\\MaP\":2:{s:1:\"a\";i:1;s:1:\"b\";i:2;}";
  $s3 = "K:3:\"map\":2:{s:1:\"a\";i:1;s:1:\"b\";i:2;}";
  $s4 = "K:3:\"Map\":2:{s:1:\"a\";i:1;s:1:\"b\";i:2;}";

  var_dump(unserialize($s1) == $m);
  var_dump(unserialize($s2) == $m);
  var_dump(unserialize($s3) == $m);
  var_dump(unserialize($s4) == $m);
}

main();
