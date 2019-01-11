<?php

function h3() {
  $x = array(1,2,3,4);
  next(&$x);
  $y = $x;
  array_pop(&$y);
  var_dump(current(&$x));
  var_dump(current(&$y));
}

<<__EntryPoint>>
function main_252() {
h3();
}
