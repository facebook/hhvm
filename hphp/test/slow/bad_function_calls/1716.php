<?php

function f() {
  $arr = array();
  sort($arr[0]);
  var_dump($arr);
  unset($arr);
  $arr = array();
  sort($arr[0],0,0,0,0,0,0,0,0);
  var_dump($arr);
}

<<__EntryPoint>>
function main_1716() {
f();
}
