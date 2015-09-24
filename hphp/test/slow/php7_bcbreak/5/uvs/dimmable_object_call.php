<?php

error_reporting(-1);

$foo = new stdclass();
$foo->someprop = array('baz' => 'myfunc');

$bar = 'someprop';

function myfunc() {
  return 'quux';
}

var_dump($foo->$bar['baz']());
