<?php

class A {
  function foo(&$test) {
    $test = 10;
  }
}

<<__EntryPoint>>
function main_1213() {
$obj = new A();
$method = 'foo';
$aa = array();
$obj->$method(&$aa[3]);
var_dump($aa);
}
