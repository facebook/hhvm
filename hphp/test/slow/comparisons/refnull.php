<?php


<<__EntryPoint>>
function main_refnull() {
  $b = null;
  $a = array(&$b);
  var_dump(in_array(null, $a, true));
}
