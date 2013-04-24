<?php
$a = array('foo' => 'original.foo');
$ref = &$a;
$foo = 'test';
extract($a, EXTR_OVERWRITE|EXTR_REFS);
$foo = 'changed.foo';
var_dump($a['foo']);
?>