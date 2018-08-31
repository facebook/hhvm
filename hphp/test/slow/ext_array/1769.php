<?php

function f($val,$key) {
  echo "k=$key v=$val\n";
}

<<__EntryPoint>>
function main_1769() {
$arr = array(0,1,2);
array_walk($arr,'f');
}
