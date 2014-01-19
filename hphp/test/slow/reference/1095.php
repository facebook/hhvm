<?php

function foo() {
  $perms = array('x' => 1);
  $t = &$perms;
  $t = $t['x'];
  unset($t);
  return $perms;
}
var_dump(foo());
