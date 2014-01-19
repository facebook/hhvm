<?php

trait T1 {
 public $xT1 = 'xT1';
 }
trait T2 {
 public $xT2 = 'xT2';
 }
trait T3 {
 public $xT3 = 'xT3';
 }
class B {
  use T1;
  public $xB = 'xB';
  use T2;
}
class A extends B {
  public $xA = 'xA';
  use T3;
}
$x = new A();
$s = serialize($x);
echo $s . "\n";
$y = unserialize($s);
var_dump($y->xT1);
var_dump($y->xT2);
var_dump($y->xT3);
var_dump($y->xA);
var_dump($y->xB);
