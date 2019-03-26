<?php

function test(&$x, &$y) {
  $x = false;
  $y .= 'hello';
  echo $x;
}

<<__EntryPoint>>
function main_1105() {
  test(&$x, &$x);
}
