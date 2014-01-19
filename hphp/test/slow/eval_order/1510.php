<?php

class Q {
  public $val;
  function __construct($v) {
    $this->val = $v;
  }
  public function blah() {
    return $this;
  }
}
class A {
  public $v;
  function set($v) {
    $this->v = $v;
    return $this;
  }
}
function id($x) {
 return $x;
 }
$x = new Q(0);
$a = id(new A)->set($x);
$x = id(new Q(1))->blah();
var_dump($a);
