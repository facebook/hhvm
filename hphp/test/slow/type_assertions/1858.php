<?php

function block() {
}
function f($x) {
  if (is_int($x) || is_array($x)) {
    var_dump($x[0]);
  }
}
function g($x) {
  $x = (array) $x;
  block();
  var_dump($x[0]);
}
f(array(10));
g(array(10));
