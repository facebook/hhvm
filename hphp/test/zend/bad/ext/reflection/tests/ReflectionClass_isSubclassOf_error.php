<?php
class A {}
$rc = new ReflectionClass('A');

var_dump($rc->isSubclassOf());
var_dump($rc->isSubclassOf('A',5));

?>
