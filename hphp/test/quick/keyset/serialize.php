<?php
// Copyright 2004-present Facebook. All Rights Reserved.

class Dtor {
  public $id;
  function __construct($id) {
    $this->id = $id;
  }
  function __destruct() {
    echo "Dtor::__destruct(): " . $this->id . "\n";
  }
}

function roundtrip($ks) {
  echo "====================================================\n";
  var_dump($ks);
  $str = serialize($ks);
  var_dump($str);
  $ks2 = unserialize($str);
  var_dump($ks2);
}

function try_serialize($val) {
  try {
    echo "====================================================\n";
    var_dump($val);
    var_dump(serialize($val));
  } catch (Exception $e) {
    echo "Serialize exception: " . $e->getMessage() . "\n";
  }
}

function try_unserialize($val) {
  try {
    echo "====================================================\n";
    var_dump($val);
    var_dump(unserialize($val));
  } catch (Exception $e) {
    echo "Unserialize exception: " . $e->getMessage() . "\n";
  }
}

function main() {
  roundtrip(keyset[]);
  roundtrip(keyset[1, 2, 3]);
  roundtrip(keyset['a', 'b', 'c']);
  roundtrip(keyset['a', 1, 'b', 2]);
  roundtrip(keyset[2, 'b', 1, 'a']);
  roundtrip(keyset[123, '123']);

  try_unserialize("k:0:{}");
  try_unserialize("k:3:{i:123;s:3:\"abc\";i:456;}");
  try_unserialize("k:2:{i:123;s:3:\"123\";}");

  // Invalid values
  try_unserialize("k:1:{b:0;}");
  try_unserialize("k:1:{d:1.23;}");
  try_unserialize("k:1:{N;}");

  // "Weak" references
  try_unserialize("k:2:{i:123;r:1;}");
  try_unserialize("a:3:{i:123;s:3:\"abc\";i:456;i:101;i:456;k:2:{r:2;r:3;}}");

  // No references
  try_unserialize("k:1:{R:1;}");
  try_unserialize("a:2:{i:123;s:3:\"abc\";i:456;k:1:{R:2;}}");
  try_unserialize("a:2:{i:123;k:1:{i:731;}i:456;R:3;}");

  // Ensure dtors run
  try_unserialize("k:2:{O:4:\"Dtor\":1:{s:2:\"id\";i:1;}O:4:\"Dtor\":1:{s:2:\"id\";i:2;}}");
}

main();
