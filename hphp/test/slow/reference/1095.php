<?php

function foo() {
  $perms = array('x' => 1);
  $t = &$perms;
  $t = $t['x'];
  unset($t);
  return $perms;
}

<<__EntryPoint>>
function main_1095() {
var_dump(foo());
}
