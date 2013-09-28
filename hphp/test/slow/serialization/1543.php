<?php

class b {
  private $foo = 1;
  private $bar = 2;
}
class b2 extends b {
  public $bar = 3;
}
$x = new b2;
$x->foo = 100;
var_dump((array)$x);
var_dump(serialize($x));
var_dump($x);
