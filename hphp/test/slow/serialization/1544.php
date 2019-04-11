<?php


<<__EntryPoint>>
function main_1544() {
  $z = null;
  $a = array(&$z);
  $a[0] = $a;
  var_dump(serialize($a));
}
