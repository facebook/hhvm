<?php
$a = array('foo' => 'original.foo');

$nonref = $a['foo'];
$ref = &$a;

extract($a, EXTR_REFS);
$a['foo'] = 'changed.foo';

var_dump($nonref);
echo "Done\n";
?>