<?php

class cls {
  static public $foo = 24;
  public $bar = 42;
}

$obj = new cls;

// Static property $foo
$fooRef = new ReflectionProperty('cls', 'foo');
$fooRef->setValue(1); // ok
var_dump($fooRef->getValue());
$fooRef->setValue($obj, 2); // fail
var_dump($fooRef->getValue());

// Non-static property $bar
$barRef = new ReflectionProperty('cls', 'bar');
$barRef->setValue(3); // fail
var_dump($barRef->getValue($obj));
$barRef->setValue($obj, 4); // ok
var_dump($barRef->getValue($obj));
