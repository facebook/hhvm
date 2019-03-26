<?php

function foo(&$perms, &$t) {
  $perms = array('x' => 1);
  $t = $t['x'];
  unset($t);
  return $perms;
}

<<__EntryPoint>>
function main_1095() {
  var_dump(foo(&$a, &$a));
}
