<?php

class X {
  public $x;
}

function test($x, &$ref) {
  $p = (new ReflectionProperty('X', 'x'))->getValue($x);
  ++$p;
  var_dump($x->x);
}

function main() {
  $x = new X;
  $x->x = 42;

  test($x, &$x->x);
}


<<__EntryPoint>>
function main_prop_by_ref() {
main();
}
