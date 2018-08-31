<?php

function myfunc() {
  return 'quux';
}


error_reporting(-1);

$foo = new stdclass();
$foo->someprop = array('baz' => 'myfunc');

$bar = 'someprop';

var_dump($foo->$bar['baz']());
