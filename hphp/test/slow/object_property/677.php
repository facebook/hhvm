<?php

class A {
  public $a = 'apple';
}

<<__EntryPoint>>
function main_677() {
$obj = new A;
var_dump(isset($obj->a), property_exists($obj, 'a'));
$obj->a = null;
var_dump(isset($obj->a), property_exists($obj, 'a'));
unset($obj->a);
var_dump(isset($obj->a), property_exists($obj, 'a'));
$obj->a = 123;
var_dump(isset($obj->a), property_exists($obj, 'a'));
$obj->a = null;
var_dump(isset($obj->a), property_exists($obj, 'a'));
}
