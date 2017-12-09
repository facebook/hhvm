<?php

class X {
  public $x;
}

function test($x) {
  $p = (new ReflectionProperty('X', 'x'))->getValue($x);
  ++$p;
  var_dump($x->x);
}

function main() {
  $x = new X;
  $x->x = 42;
  $y = &$x->x;

  test($x);
}

main();
