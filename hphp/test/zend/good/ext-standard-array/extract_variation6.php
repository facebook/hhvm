<?php

/* EXTR_REFS as second Argument */
$a = array ('foo' => 'aaa');
var_dump ( extract($a, EXTR_REFS));
var_dump($foo);

$b = $a;
$b['foo'] = 'bbb';
var_dump ( extract($a, EXTR_REFS));
var_dump($foo);
var_dump($a);

echo "Done\n";
?>