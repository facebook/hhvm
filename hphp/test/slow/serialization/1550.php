<?php

class X {
  private $str;
  private $arr;
  private $obj;
  function foo() {
    $this->str = 'hello';
    $this->arr = array(1,2,3);
    $this->obj = $this;
  }
}
function test() {
  $x = new X;
  $s = serialize($x);
  $x = unserialize($s);
  var_dump($x);
}
test();
