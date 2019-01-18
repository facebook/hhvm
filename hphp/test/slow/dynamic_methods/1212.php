<?php

function t($x) {
  var_dump($x);
}
class z {
  function q() {
    $x = array(1,2,3);
    array_map(array('z', 'p'), $x);
  }
  function p($x) {
    var_dump($x);
  }
}

<<__EntryPoint>>
function main_1212() {
$x = array(1,2,3);
array_map('t', $x);
$m = new z();
$m->q();
}
