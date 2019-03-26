<?php

function f(&$b, &$c) {
  $a = array(123);
  $b = $a;
  $d = new stdClass();
  $v = array(&$a, &$b, &$c, $d, $d);
  $s = serialize($v);
  echo $s;
}

<<__EntryPoint>>
function main_1864() {
  f(&$b, &$b);
}
