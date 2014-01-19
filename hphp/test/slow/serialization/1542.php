<?php

class t {
  public $foo = 10;
  protected $bar = 20;
  private $derp = 30;
}
class t2 extends t {
  private $derp2 = 40;
  protected $bar2 = 50;
}
$x = new t;
print_r($x);
var_dump($x);
echo serialize($x) . '
';
$x2 = new t2;
print_r($x2);
var_dump($x2);
echo serialize($x2) . '
';
