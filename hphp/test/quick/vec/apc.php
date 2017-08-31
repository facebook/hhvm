<?php
// Copyright 2004-present Facebook. All Rights Reserved.

class SomeClass {
  public $val;
}

class Sleep {
  public $val;

  function __construct($val) {
    $this->val = $val;
  }

  function __sleep() {
    echo "Sleep...\n";
    return [];
  }
}

class Wakeup {
  function __wakeup() {
    echo "Wakeup...\n";
    $this->val = 12345;
  }
}

class WakeupThrow {
  function __wakeup() {
    throw new Exception("Wakeup exception");
  }
}

class Dtor {
  public $val;

  function __construct($val) {
    $this->val = $val;
  }

  function __destruct() {
    echo "Dtor... " . $this->val . "\n";
  }
}

function get_count() {
  $count = apc_fetch("count");
  if (!$count) {
    $count = 0;
  }
  $count++;
  apc_store("count", $count);
  return $count;
}

function read() {
  var_dump(apc_fetch("val1"));
  var_dump(apc_fetch("val2"));
  var_dump(apc_fetch("val3"));
  var_dump(apc_fetch("val4"));
  var_dump(apc_fetch("val5"));
  var_dump(apc_fetch("val6"));
  var_dump(apc_fetch("val7"));
  var_dump(apc_fetch("val8"));

  try {
    var_dump(apc_fetch("val9"));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  try {
    var_dump(apc_fetch("val10"));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  var_dump(apc_fetch("val11"));
}

function write($count) {
  apc_store("val1", vec[]);
  apc_store("val2", vec[1, 2, 3]);
  apc_store("val3", vec["a", "b", "C", "D"]);
  apc_store("val4", vec[$count, $count]);
  apc_store("val5", vec[new stdclass, new stdclass]);

  $cls = new SomeClass;
  $cls->val = vec[vec[], vec[$count], vec[$count, $count+1]];
  apc_store("val6", $cls);

  $cls2 = new SomeClass;
  $v = vec[$cls2];
  $cls2->val = $v;
  apc_store("val7", $cls2);

  apc_store("val8", vec[new Wakeup]);
  apc_store("val9", vec[new WakeupThrow]);
  apc_store("val10", vec[new Dtor(1), new WakeupThrow, new Dtor(2)]);
  apc_store("val11", vec[new Sleep(123)]);
}

function main() {
  $count = get_count();
  echo "Count: $count\n";
  read();
  write($count);
  read();
}
main();
