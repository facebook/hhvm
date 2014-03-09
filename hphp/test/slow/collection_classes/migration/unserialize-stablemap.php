<?hh

// Test that we can handle unserialization of StableMaps with
// and without namespaces.

function main() {
  $sm = Map {'a' => 1, 'b' => 2};

  // unserialize o serialize == id
  var_dump(unserialize(serialize($sm)) == $sm);

  // Namespaced StableMap.
  $nsm = "K:9:\"StableMap\":2:{s:1:\"a\";i:1;s:1:\"b\";i:2;}";
  var_dump(unserialize($nsm) == $sm);

  // Nested namespaced StableMap.
  $nested = "K:9:\"StableMap\":2:{i:0;K:9:\"StableMap\":0:{}" .
            "i:1;K:9:\"StableMap\":1:{s:1:\"a\";i:2;}}";
  var_dump(unserialize($nested) ==
           Map {0 => Map {}, 1 => Map {'a' => 2}});

  // O format
  $o1 = "O:9:\"StableMap\":0:{}";
  $o2 = "O:9:\"StableMap\":0:{}";
  var_dump(unserialize($o1) == Map {});
  var_dump(unserialize($o2) == Map {});

  // Unserialization is case-insensitive.
  $s1 = "K:9:\"stablemap\":2:{s:1:\"a\";i:1;s:1:\"b\";i:2;}";
  $s2 = "K:9:\"StableMaP\":2:{s:1:\"a\";i:1;s:1:\"b\";i:2;}";
  $s3 = "K:9:\"stablemap\":2:{s:1:\"a\";i:1;s:1:\"b\";i:2;}";
  $s4 = "K:9:\"StableMap\":2:{s:1:\"a\";i:1;s:1:\"b\";i:2;}";

  var_dump(unserialize($s1) == $sm);
  var_dump(unserialize($s2) == $sm);
  var_dump(unserialize($s3) == $sm);
  var_dump(unserialize($s4) == $sm);
}

main();
