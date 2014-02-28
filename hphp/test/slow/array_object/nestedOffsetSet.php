<?php

class ThisIsAnArray extends ArrayObject { }

$foo = new ThisIsAnArray();
$foo['foo'] = 'foo';
$foo['arr'] = [];
$foo['arr']['ta'] = 'bar';
$foo['arr']['l1']['l2'] = 'baz';

var_dump($foo['foo']);
var_dump($foo['arr']['ta']);
var_dump($foo['arr']['l1']['l2']);
