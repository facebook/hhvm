<?php

function baz(&$x) {
  var_dump($x);
}

class A {
  private $x;
  private $z;
  function foo() {
    $this->x = $this;
  }
  function bar() {
    $y = $this->x->x->x->x->x;
    baz($y);
    $y = $this->x->x->x->x->x;
    baz($y);
    baz($this->z);
  }

  public function what() {
    $this->foo();
    $this->bar();
    unset($this->x->x);
    $this->bar();
  }
}

function foo() {
  $x = new A();
  $x->what();
  $x->what();
}
foo();
