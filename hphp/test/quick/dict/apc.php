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
  apc_store("val1", dict[]);
  apc_store("val2", dict["a" => 1, "b" => 2, "c" => 3]);
  apc_store("val3", dict[4 => "a", 5 => "b", 6 => "C", 7 => "D"]);
  apc_store("val4", dict[$count => 999, 999 => $count]);
  apc_store("val5", dict[0 => new stdclass, 1 => new stdclass]);

  $cls = new SomeClass;
  $cls->val = dict[50 => dict[], "50" => dict[100 => $count],
                   $count => dict[101 => $count, 102 => $count+1]];
  apc_store("val6", $cls);

  $cls2 = new SomeClass;
  $d = dict["cls" => $cls2];
  $cls2->val = $d;
  apc_store("val7", $cls2);

  apc_store("val8", dict[123 => new Wakeup]);
  apc_store("val9", dict["456" => new WakeupThrow]);
  apc_store("val10", dict[1 => new Dtor(1), 2 => new WakeupThrow, 3 => new Dtor(2)]);
  apc_store("val11", dict['abc' => new Sleep(123)]);
}

function main() {
  $count = get_count();
  echo "Count: $count\n";
  read();
  write($count);
  read();
}
main();
