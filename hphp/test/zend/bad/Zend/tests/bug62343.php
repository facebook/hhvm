<?php
class a { }
class_alias("a", "b");
$c = get_declared_classes();
var_dump(end($c));
var_dump(prev($c));
?>