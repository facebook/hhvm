<?php

class X {
  public $x = 0;
  function y($u,&$a) {
    $a++;
  }
}
;
if (0) {
 class X{
}
 }
function f() {
}
function test() {
  $x = new X;
  $x->y(f(),$x->x);
  var_dump($x);
  $x->y(0,$x->x);
  var_dump($x);
}
test();
