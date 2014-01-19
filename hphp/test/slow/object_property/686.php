<?php

class A {
  private $pri = 'a-pri';
  protected $pro = 'a-pro';
  function bar() {
    var_dump($this->pri);
    $pri = $q ? 'zz' : 'p'.'ri';
    var_dump($this->$pri);
  }
  function bar2() {
    var_dump($this->pro);
  }
}
class B extends A {
  private $pri = 'b-pri';
  function bar() {
    parent::bar();
    var_dump($this->pri);
  }
}
$obj = new B();
 $obj->bar();
class C extends B {
  public $pri = 'c-pri';
  public $pro = 'c-pro';
  function bar2() {
    parent::bar2();
    var_dump($this->pro);
  }
}
$obj = new C;
 $obj->bar();
 $obj->bar2();
var_dump(serialize($obj));
class Base {
  protected $pro = 1;
  private $pri = 'base-pri';
  function q0() {
    var_dump($this->pri);
  }
}
class R extends Base {
  public $rpub = 1;
  protected $pro = 2;
  private $pri = 'r-pri';
  function q() {
    var_dump($this->pri);
  }
}
class D extends R {
  public $dpub = 'd';
  protected $pro = 'pro';
  private $pri = 'd-pri';
  function qq() {
    var_dump($this->pri);
  }
}
class DD extends D {
  private $pri = 'dd-pri';
  function qqq() {
    var_dump($this->pri);
  }
}
if (false) {
  class R{
}
}
$d = new D;
$d->qq();
$d->q();
$d->q0();
$d = new DD;
$d->qqq();
$d->qq();
$d->q();
$d->q0();
