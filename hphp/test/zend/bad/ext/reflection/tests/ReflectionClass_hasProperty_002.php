<?php
class C {
	public $a;
}

$rc = new ReflectionClass("C");
echo "Check invalid params:\n";
var_dump($rc->hasProperty());
var_dump($rc->hasProperty("a", "a"));
var_dump($rc->hasProperty(null));
var_dump($rc->hasProperty(1));
var_dump($rc->hasProperty(1.5));
var_dump($rc->hasProperty(true));
var_dump($rc->hasProperty(array(1,2,3)));
var_dump($rc->hasProperty(new C));
?>
