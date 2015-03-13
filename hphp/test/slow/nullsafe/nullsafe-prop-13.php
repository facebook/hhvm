<?hh // strict

function byRef(&$x) {}

function test() {
  $x = null;
  byRef($x?->y); // error
}

test();
