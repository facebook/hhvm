<?php

class A {
  function hehe() { return 2; }
}

class B {
  function hehe() { return "asd"; }
}

class C {
  function& __call($x, $y) { global $x; return $x; }
}

$x = 'heh';

function bar($z) {
  return $z->hehe();
}

var_dump(bar(new C));
