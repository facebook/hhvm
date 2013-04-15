<?php

class C {
  public static $x;
}

function f(&$arg) {}

C::$x[0][][][][] = 1;
var_dump(C::$x);
var_dump(C::$x[0][0][0][0][0]);
f(C::$x[][]);
var_dump(C::$x);

