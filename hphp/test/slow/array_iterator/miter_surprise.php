<?php

function foo($x) {
  set_time_limit(1);
  foreach ($x as &$v) {
    $x = array(0);
  }
}

<<__EntryPoint>>
function main_miter_surprise() {
foo(array(1,2,3,4));
}
