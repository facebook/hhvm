<?php

function g() {
  throw new Exception("Fooception");
  return "Oh uh...";
}

function f($val = g()) {
  echo "val = ";
  var_dump($val);
}

try {
  f();
} catch (Exception $e) {
  echo "Caught exception: {$e->getMessage()}\n";
}
