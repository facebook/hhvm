<?php

if (isset($g)) {
  interface I {
 const FOO = 1;
 }
  class C {
 const FOO = 3;
 }
}
 else {
  interface I {
 const FOO = 2;
 }
  class C {
 const FOO = 4;
 }
}
class X {
  function foo($x = C::FOO, $y = I::FOO) {
}
}
function test() {
  $x = new ReflectionMethod('X', 'foo');
  foreach ($x->getParameters() as $p) {
    var_dump($p->getDefaultValue());
  }
}
function fiz($c) {
  var_dump($c::FOO);
}
fiz('I');
fiz('C');
test();
