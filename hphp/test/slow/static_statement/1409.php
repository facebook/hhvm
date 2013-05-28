<?php

class c {
  public $q = 20;
  function x() {
    $foo = 20;
    static $foo;
    $foo = $this->q;
    echo $foo;
  }
  function y() {
    static $foo = 20;
    $foo++;
    echo $foo;
  }
  static function sf() {
    static $foo = 0;
    $foo++;
    echo $foo;
  }
}
class d extends c {
  public $q = 30;
}
$x = new c();
$x->x();
$x->y();
$x->y();
$x->y();
$x->y();
$x = new d();
$x->x();
$x->y();
$x->y();
$x->y();
c::sf();
c::sf();
c::sf();
d::sf();
d::sf();
d::sf();
