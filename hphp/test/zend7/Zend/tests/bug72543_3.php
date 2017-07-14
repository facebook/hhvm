<?php
$x = new stdClass;
$x->a = 1;
$ref =& $x->a;
unset($ref);
var_dump($x->a + ($x->a = 2));
?>
