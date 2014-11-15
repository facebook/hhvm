<?php

define('MY_CONST', 123);

function f($val = (MY_CONST === 123) ? "Foo" : "Bar") {
  echo "val = ";
  var_dump($val);
}

f();
