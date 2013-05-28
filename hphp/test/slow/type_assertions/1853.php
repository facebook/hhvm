<?php

function g(&$x) {
 var_dump($x);
 }
function f($x) {
  if (is_array($x)) {

    var_dump($x);
    var_dump($x[0]);
    var_dump($x[0][1]);
  }
  if (is_array($x) && $x) {

    g($x);
    g($x[0]);
    g($x[0][1]);
  }
}
f(null);
f(array());
f(array(0, 1));
f(array(array(1 => 1)));
