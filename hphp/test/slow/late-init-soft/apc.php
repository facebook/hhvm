<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function error_handler($errno, $errstr) {
  echo $errstr . "\n";
}

class A {
  public $x = 123;
  <<__SoftLateInit>> public $y;
  public $z = 'abc';
}

function test() {
  set_error_handler('error_handler');

  HH\set_soft_late_init_default(vec[]);

  $a = new A();
  apc_store('a-key1', $a);
  HH\set_soft_late_init_default(123);
  var_dump($a->y);
  apc_store('a-key2', $a);
}

test();
