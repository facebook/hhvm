<?php

function x($str) {
  // tvCoerceParamToStringInPlace
  return mb_strtolower($str);
}

class Foo {
   // will throw an exception
  function __toString() {
    return 2;
  }
}

var_dump(x(new Foo));
