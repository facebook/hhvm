<?php

CONST BAZ = 0;

class Foo {
  CONST BAR = 1;
  VAR $x = 2;
}

function f($x) {
  foreach ($x AS $k => $v) {
    if ($v instanceOf Foo) {}
  }
}
