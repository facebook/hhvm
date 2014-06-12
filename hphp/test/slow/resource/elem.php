<?php

error_reporting(-1);

function lol() { return STDIN; }
function foo() {
  $x = lol();
  $y = $x[0]['id'];
  var_dump($x);
  var_dump($x[1] = 'hi');
  return $y;
}

var_dump(foo());
