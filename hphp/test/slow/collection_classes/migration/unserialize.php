<?hh

// Test that we can handle unserialization of Vectors with
// and without namespaces.

function main() {
  $v = Vector {1, 2, 3};

  // unserialize o serialize == id
  var_dump(unserialize(serialize($v)) == $v);

  // Namespaced vector.
  $nsv = "V:9:\"HH\\Vector\":3:{i:1;i:2;i:3;}";
  var_dump(unserialize($nsv) == $v);

  // Nested namespaced vectors.
  $nested = "V:9:\"HH\\Vector\":2:{V:9:\"HH\\Vector\":0:{}" .
            "V:9:\"HH\\Vector\":2:{i:1;i:2;}}";
  var_dump(unserialize($nested) == Vector {Vector {}, Vector {1, 2}});

  // O format
  $o1 = "O:6:\"Vector\":0:{}";
  $o2 = "O:9:\"HH\\Vector\":0:{}";
  var_dump(unserialize($o1) == Vector {});
  var_dump(unserialize($o2) == Vector {});

  // Unserialization is case-insensitive.
  $s1 = "V:9:\"hh\\vector\":3:{i:1;i:2;i:3;}";
  $s2 = "V:9:\"HH\\VecTor\":3:{i:1;i:2;i:3;}";
  $s3 = "V:6:\"vector\":3:{i:1;i:2;i:3;}";
  $s4 = "V:6:\"Vector\":3:{i:1;i:2;i:3;}";

  var_dump(unserialize($s1) == $v);
  var_dump(unserialize($s2) == $v);
  var_dump(unserialize($s3) == $v);
  var_dump(unserialize($s4) == $v);
}

main();
