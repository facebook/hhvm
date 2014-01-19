<?php
class A {}
foreach(($a=(object)new A()) as $v);
var_dump($a); // UNKNOWN:0
?>