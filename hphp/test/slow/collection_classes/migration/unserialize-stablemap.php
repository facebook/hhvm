<?hh

// Test that we can handle unserialization of StableMaps with
// and without namespaces.

function main() {
  $m = StableMap {'a' => 1, 'b' => 2};

  // unserialize o serialize == id
  var_dump(unserialize(serialize($m)) == $m);

  // Namespaced StableMap.
  $nsm = "K:12:\"HH\\StableMap\":2:{s:1:\"a\";i:1;s:1:\"b\";i:2;}";
  var_dump(unserialize($nsm) == $m);

  // Nested namespaced StableMap.
  $nested = "K:12:\"HH\\StableMap\":2:{i:0;K:12:\"HH\\StableMap\":0:{}" .
            "i:1;K:12:\"HH\\StableMap\":1:{s:1:\"a\";i:2;}}";
  var_dump(unserialize($nested) ==
           StableMap {0 => StableMap {}, 1 => StableMap {'a' => 2}});

  // O format
  $o1 = "O:9:\"StableMap\":0:{}";
  $o2 = "O:12:\"HH\\StableMap\":0:{}";
  var_dump(unserialize($o1) == StableMap {});
  var_dump(unserialize($o2) == StableMap {});

  // Unserialization is case-insensitive.
  $s1 = "K:12:\"hh\\stablemap\":2:{s:1:\"a\";i:1;s:1:\"b\";i:2;}";
  $s2 = "K:12:\"HH\\StableMaP\":2:{s:1:\"a\";i:1;s:1:\"b\";i:2;}";
  $s3 = "K:9:\"stablemap\":2:{s:1:\"a\";i:1;s:1:\"b\";i:2;}";
  $s4 = "K:9:\"StableMap\":2:{s:1:\"a\";i:1;s:1:\"b\";i:2;}";

  var_dump(unserialize($s1) == $m);
  var_dump(unserialize($s2) == $m);
  var_dump(unserialize($s3) == $m);
  var_dump(unserialize($s4) == $m);
}

main();
