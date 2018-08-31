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


<<__EntryPoint>>
function main_coerce_with_error_in_tostring() {
var_dump(x(new Foo));
}
