<?php
$foo = 'bar';
var_dump(strval($foo));
define('FOO', 'BAR');
var_dump(strval(FOO));
var_dump(strval('foobar'));
var_dump(strval(1));
var_dump(strval(1.1));
var_dump(strval(true));
var_dump(strval(false));
var_dump(strval(array('foo')));
?>
