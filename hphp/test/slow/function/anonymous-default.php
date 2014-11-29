<?php

function f($val = (function() { return strtoupper("Foo"); })()) {
  echo "val = ";
  var_dump($val);
}

f();
