<?php


<<__EntryPoint>>
function main_1544() {
  $a = array();
  $a[0] = array(&$a[0]);
  var_dump(serialize($a));
}
