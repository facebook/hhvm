<?php

class FooException extends Exception {
  protected $code = 42;
  protected $message = 'test';
}

$e = new FooException();
var_dump($e->getMessage());
var_dump($e->getCode());

$e = new FooException('hello', 123);
var_dump($e->getMessage());
var_dump($e->getCode());
