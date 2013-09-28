<?php

class X {
  public $propX;
  function baz() {
    echo 'X';
  }
  function foo() {
    if ($this instanceof Y) {
      $this->bar();
      $this->baz();
      var_dump($this->propX);
      return $this->propY;
    }
    return null;
  }
}
class Y extends X {
  public $propY;
  function bar(){
    echo 'Y';
  }
}
$y = new Y;
$y->propX = 16;
$y->propY = 32;
var_dump($y->foo());
class A1 {
  public $a1prop;
  function a1method() {
    return 0;
  }
  function doStuff() {
    if ($this instanceof D1) {
      var_dump($this->d1prop);
      var_dump($this->d1method());
    }
 else if ($this instanceof C1) {
      var_dump($this->c1prop);
      var_dump($this->c1method());
    }
 else if ($this instanceof B1) {
      var_dump($this->b1prop);
      var_dump($this->b1method());
    }
 else if ($this instanceof A1) {
      var_dump($this->a1prop);
      var_dump($this->a1method());
    }
  }
}
class B1 extends A1 {
  public $b1prop;
  function b1method() {
    return 1;
  }
}
if (rand(0, 1)) {
  class C1 extends B1 {
    public $c1prop;
    function c1method() {
      return 2;
    }
  }
}
 else {
  class C1 extends B1 {
    public $c1prop;
    function c1method() {
      return 2;
    }
  }
}
class D1 extends C1 {
  public $d1prop;
  function d1method() {
    return 3;
  }
}
$a1 = new A1;
$a1->a1prop = 0;
$a1->doStuff();
$b1 = new B1;
$b1->b1prop = 1;
$b1->doStuff();
$c1 = new C1;
$c1->c1prop = 2;
$c1->doStuff();
$d1 = new D1;
$d1->d1prop = 3;
$d1->doStuff();
