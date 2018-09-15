<?php

function gen(&$x) {
  $x = 1;
  yield 1;
  $x = 2;
  yield 3;
  $x = 4;
}
function test() {
  $x = 0;
  foreach (gen(&$x) as $y) {
    var_dump($y);
  }
  var_dump($x);
}

<<__EntryPoint>>
function main_2186() {
test();
}
