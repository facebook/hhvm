<?php

function f($val = (MY_CONST === 123) ? "Foo" : "Bar") {
  echo "val = ";
  var_dump($val);
}


<<__EntryPoint>>
function main_ternary_default() {
define('MY_CONST', 123);

f();
}
