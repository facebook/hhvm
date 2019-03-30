<?php

class X {
  public $x = 0;
  function y($u,&$a) {
    $a++;
  }
}
function f() {
}
function test() {
  $x = new X;
  $x->y(f(),&$x->x);
  var_dump($x);
  $x->y(0,&$x->x);
  var_dump($x);
}

<<__EntryPoint>>
function main_688() {
  if (0) {
    include '688.inc';
  }
  test();
}
