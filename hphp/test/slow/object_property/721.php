<?php

class foo {
  function __get($property) {
    return $property; 
  }
}

$object = new foo;
$attribute = NULL;
var_dump($object->$attribute);
