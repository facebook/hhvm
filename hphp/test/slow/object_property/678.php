<?php

class A {
  private $prop = 'test';

  function __get($name) {
    return $this->$name;
  }
}

$obj = new A();
var_dump($obj->prop);
