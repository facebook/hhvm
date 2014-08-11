<?php
class A {}
$rc = new ReflectionClass('A');

var_dump($rc->isSubclassOf('X'));

?>
