<?php

Trait T {
 function bar() {
 yield 1;
 }
 }
class X {
 use T;
 }
function test() {
  $r = new ReflectionClass('X');
  foreach ($r->getMethods() as $m) {
    var_dump($m->name);
  }
}
test();
