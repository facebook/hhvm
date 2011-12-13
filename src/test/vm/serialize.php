<?php

class A {
  public $a = 1;
  private $b = "hello";
  protected $c = array(1, 2);
}

class B extends A {
  public $b = 0;
}

$a = new B;
$b = serialize($a);
var_dump($b);
$c = unserialize($b);
var_dump($c);
