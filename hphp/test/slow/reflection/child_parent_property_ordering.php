<?php
class A {
  protected $extension = "html";
  protected $apple = 3;
  protected $foo = null;
  private $orange = null;
}


class B extends A {
  public $foo = null;
  protected $bar = array();
}

$class = new ReflectionClass("B");
var_dump($class->getProperties());
