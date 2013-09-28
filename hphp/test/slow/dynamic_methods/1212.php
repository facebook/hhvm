<?php

function t($x) {
  var_dump($x);
}
$x = array(1,2,3);
array_map('t', $x);
class z {
  function q() {
    $x = array(1,2,3);
    array_map(array('self', 'p'), $x);
  }
  function p($x) {
    var_dump($x);
  }
}
$m = new z();
$m->q();
