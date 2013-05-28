<?php

class X {
}
class Y extends X {
 public $foo;
 }
function foo() {
  $x = new Y;
  $x && var_dump($x->foo);
  $x = new X;
  var_dump($x);
}
foo();
