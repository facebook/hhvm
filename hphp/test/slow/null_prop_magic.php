<?php

class Foo {
  public function __get($prop) { echo "__get($prop)\n"; }
  public function __set($prop, $val) { echo "__set($prop, $val)\n"; }
  public function __isset($prop) { echo "__isset($prop)\n"; }
  public function __unset($prop) { echo "__unset($prop)\n"; }
}

$obj = new Foo;
$prop = "\0myprop";

$obj->$prop = "should work";
$_ = $obj->$prop;
isset($obj->$prop);
unset($obj->$prop);
