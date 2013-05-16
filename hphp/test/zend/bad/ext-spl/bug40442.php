<?php
$a = new ArrayObject();
$a->offsetSet('property', 0);
var_dump($a->offsetExists('property'));
?>
===DONE===