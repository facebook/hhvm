<?php

function f(&$val,$key) {
  echo "k=$key v=$val\n";
  $val = $val + 1;
}

<<__EntryPoint>>
function main_1768() {
$arr = array(0,1,2);
array_walk($arr,'f');
var_dump($arr);
}
