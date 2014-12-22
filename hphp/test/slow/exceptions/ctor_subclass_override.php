<?php

class FooException extends Exception {
  function __construct() {}
}

$e = new FooException('test', 42);
var_dump($e->getMessage());
var_dump($e->getCode());
