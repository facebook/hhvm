<?php

error_reporting(-1);

$foo = new stdclass();
$foo->someprop = array('baz' => 'quux');

$bar = 'someprop';

var_dump($foo->$bar['baz']);
