<?php
// Copyright 2004-present Facebook. All Rights Reserved.

class NoisyClass {
  function __sleep() {
    echo "NoisyClass::__sleep()\n";
    return [];
  }
  function __wakeup() {
    echo "NoisyClass::__wakeup()\n";
    $this->val = dict["abcd" => 12345];
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

function roundtrip($d) {
  echo "====================================================\n";
  var_dump($d);
  $str = serialize($d);
  var_dump($str);
  $d2 = unserialize($str);
  var_dump($d2);
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
  roundtrip(dict[]);
  roundtrip(dict[1 => 'a', 2 => 'b', 3 => 'c']);
  roundtrip(dict['a' => 1, 'b' => 2, 'c' => 3]);
  roundtrip(dict['abc' => new stdclass, 'def' => true,
                 123 => 1.23, 456 => null,
                 789 => dict['a' => 1, 'b' => 2],
                 'ghi' => keyset[123, "abc"],
                 'jkl' => vec[1, 2, 3, 4]]);
  roundtrip(dict['123' => 1, 123 => 2]);
  roundtrip(dict[1 => new NoisyClass]);

  try_unserialize("D:0:{}");
  try_unserialize("D:2:{i:123;s:2:\"ab\";s:2:\"cd\";i:456;}");
  try_unserialize("D:2:{i:123;i:555;s:3:\"123\";i:666;}");

  // Duplicate keys
  try_unserialize("D:2:{i:1;i:123;i:1;i:456;}");
  try_unserialize("D:2:{i:1;O:4:\"Dtor\":1:{s:2:\"id\";i:1;}i:1;i:456;}");

  // Recursive data structures
  try_unserialize("D:1:{i:1;O:8:\"stdClass\":1:{s:3:\"val\";a:1:{i:1;r:2;}}}");
  $recursive_obj = new stdclass;
  $recursive_dict = dict[1 => $recursive_obj];
  $recursive_obj->val = $recursive_dict;
  roundtrip($recursive_obj);
  roundtrip($recursive_dict);

  // "Weak" references
  try_unserialize("D:2:{s:3:\"abc\";v:1:{i:456;}s:3:\"def\";r:2;}");
  try_unserialize("D:2:{s:3:\"abc\";s:3:\"zyx\";r:2;i:123;}");

  // Invalid keys
  try_unserialize("D:1:{b:0;i:123}");
  try_unserialize("D:1:{a:0:{};i:123}");
  try_unserialize("D:2:{i:123;a:1:{i:0;s:3:\"abc\";}R:3;i:456;}");
  try_unserialize("D:2:{i:123;a:1:{i:0;b:1;}r:3;i:456;}");

  // No references
  try_unserialize("D:2:{i:1;i:2;i:3;R:2}");
  try_unserialize("D:2:{i:1;i:789;i:2;a:1:{i:123;R:2;}}");

  // References in non-Hack array sub-arrays are okay.
  try_unserialize("D:2:{i:1;a:1:{i:0;i:123;}i:2;a:1:{i:0;R:3;}}");

  try_serialize(dict[1 => new SleepThrow]);
  try_unserialize("D:1:{i:123;O:11:\"WakeupThrow\":0:{}}");

  // Ensure propert dtors run
  try_unserialize("D:3:{i:1;O:4:\"Dtor\":1:{s:2:\"id\";i:1;}i:2;O:11:\"WakeupThrow\":0:{}i:3;O:4:\"Dtor\":1:{s:2:\"id\";i:2;}}");
  try_unserialize("D:3:{i:1;O:4:\"Dtor\":1:{s:2:\"id\";i:1;}b:0;s:3:\"abc\";i:3;O:4:\"Dtor\":1:{s:2:\"id\";i:2;}}");
}

main();
