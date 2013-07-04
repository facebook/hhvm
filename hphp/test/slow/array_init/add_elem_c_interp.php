<?php ;

class X {
  public $foo;

  function test() {
    $a = array(
      null => $this->foo,
    );
    return $a;
  }
}

$x = new X;
var_dump($x->test());
