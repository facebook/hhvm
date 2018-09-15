<?php

function aa() { return 'a'; }
function heh() { return array('a' => aa()); }
function x() {
  $x = heh();
  $r = $x[null] = 2;
  var_dump($r);
  var_dump($x);
}

<<__EntryPoint>>
function main_array_037() {
x();
}
