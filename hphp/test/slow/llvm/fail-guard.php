<?php


function main($i, $j) {
  return $i + $j;
}

<<__EntryPoint>>
function main_fail_guard() {
var_dump(main(1, 2));
var_dump(main(1.1, 2.2));
}
