<?php

class Foo {
  public $d = 4;
  protected $e = 5;
  private $f = 6;

  public function __debugInfo() {
    return ['a'=>1, "\0*\0b"=>2, "\0Foo\0c"=>3];
  }
}

class Bar {
  public $val = 123;

  public function __debugInfo() {
    return null;
  }
}

$f = new Foo;
var_dump($f);

$b = new Bar;
var_dump($b);