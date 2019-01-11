<?php

function f() {
  $arr = array(1 => 2, '1d' => 3);
  extract(&$arr);
  $vars = get_defined_vars();
 asort(&$vars);
 var_dump($vars);
}

<<__EntryPoint>>
function main_1154() {
f();
}
