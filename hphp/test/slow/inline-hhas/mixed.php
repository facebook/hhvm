<?php

function f($x) {
  $y = $x * 100;
  var_dump(hh\asm('
    CGetL $x
    CGetL $y
    Add
    SetL $z
    PopC
    # we dont leave anything on the eval stack,
    # so the emitter should push null
  '));
  return $z;
}


var_dump(f(4));
