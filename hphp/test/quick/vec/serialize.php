<?php
// Copyright 2004-present Facebook. All Rights Reserved.

class NoisyClass {
  function __sleep() {
    echo "NoisyClass::__sleep()\n";
    return [];
  }
  function __wakeup() {
    echo "NoisyClass::__wakeup()\n";
    $this->val = vec[12345];
  }
}

class SleepThrow {
  function __sleep() {
    throw new Exception("Sleep exception");
  }
}

class WakeupThrow {
  function __wakeup() {
    throw new Exception("Wakeup exception");
  }
}

class Dtor {
  public $id;
  function __construct($id) {
    $this->id = $id;
  }
  function __destruct() {
    echo "Dtor::__destruct(): " . $this->id . "\n";
  }
}

function roundtrip($v) {
  echo "====================================================\n";
  var_dump($v);
  $str = serialize($v);
  var_dump($str);
  $v2 = unserialize($str);
  var_dump($v2);
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
  roundtrip(vec[]);
  roundtrip(vec[1, 2, 3]);
  roundtrip(vec['a', 'b', 'c', 'd', 'e', 'f', 'g']);
  roundtrip(vec['a', 'b', 100, 200, 'c', 500]);
  roundtrip(vec[1, 1, 1, 'a', 'a', 'a']);
  roundtrip(vec[123, "123"]);
  roundtrip(vec[new stdclass(), true, false, 1.23, null]);
  roundtrip(vec[vec[], vec[100, 200], vec["key1", "key2", "key3"]]);
  roundtrip(vec[[], [111, 222],
                dict['a' => 50, 'b' => 60, 'c' => 70],
                keyset["abc", 123, "def", 456]]);
  roundtrip(vec[new NoisyClass]);

  try_unserialize("v:0:{}");
  try_unserialize("v:4:{i:123;s:3:\"abc\";b:0;N;}");
  try_unserialize("v:2:{i:123;s:3:\"123\";}");

  // "Weak" references
  try_unserialize("v:2:{v:1:{i:123;}r:2;}");

  // No references
  try_unserialize("v:2:{v:1:{i:123;}R:2;}");
  try_unserialize("v:2:{i:123;a:1:{i:1;R:2;}}");

  // References in non-Hack array sub-arrays are okay.
  try_unserialize("v:2:{a:1:{i:0;i:123;}a:1:{i:0;R:3;}}");

  // Recursive data structures
  try_unserialize("v:1:{O:8:\"stdClass\":1:{s:3:\"val\";v:1:{r:2;}}}");
  $recursive_obj = new stdclass;
  $recursive_vec = vec[$recursive_obj];
  $recursive_obj->val = $recursive_vec;
  roundtrip($recursive_obj);
  roundtrip($recursive_vec);

  try_serialize(vec[new SleepThrow]);
  try_unserialize("v:1:{O:11:\"WakeupThrow\":0:{}}");

  // Ensure dtors of already unserialized elements run
  try_unserialize("v:5:{O:4:\"Dtor\":1:{s:2:\"id\";i:1;}O:4:\"Dtor\":1:{s:2:\"id\";i:2;}O:4:\"Dtor\":1:{s:2:\"id\";i:3;}O:11:\"WakeupThrow\":0:{}O:4:\"Dtor\":1:{s:2:\"id\";i:4;}}");
  try_unserialize("v:3:{v:1:{i:123;}R:2;O:4:\"Dtor\":1:{s:2:\"id\";i:1;}}");
  try_unserialize("v:3:{O:4:\"Dtor\":1:{s:2:\"id\";i:1;}v:1:{i:123;}R:3;}");
}

main();
