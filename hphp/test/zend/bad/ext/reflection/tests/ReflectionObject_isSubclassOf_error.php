<?php
class A {}
$ro = new ReflectionObject(new A);

var_dump($ro->isSubclassOf());
var_dump($ro->isSubclassOf('A',5));
var_dump($ro->isSubclassOf('X'));

?>
