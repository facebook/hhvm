<?php

// disable array -> "Array" conversion notice
error_reporting(error_reporting() & ~E_NOTICE);

class bar {
  function __toString() {
    return "bar string";
  }
}

class foo {
  function __get($property) {
    return $property;
  }
}

$object = new foo;

$attribute = NULL;
var_dump($object->$attribute);
$attribute = true;
var_dump($object->$attribute);
$attribute = false;
var_dump($object->$attribute);
$attribute = 1.1;
var_dump($object->$attribute);
$attribute = array();
var_dump($object->$attribute);
$attribute = "\0key";
var_dump($object->$attribute);
$attribute = new bar;
var_dump($object->$attribute);
$attribute = new stdclass;
var_dump($object->$attribute);
