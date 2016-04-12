<?php

error_reporting(-1);

$foo = 'myvar';
$myvar = array('bar' => array('baz' => 'quux'));

var_dump($$foo['bar']['baz']);
