<?php

abstract class Foo {
  public $a;
  protected $b;
  private $c = 456;
  static protected $d = 789;

  abstract function bar($bar, $baz = 123);
  static final protected function baz(stdClass $a = null) {}
}

echo (string)(new ReflectionClass('Foo'));
