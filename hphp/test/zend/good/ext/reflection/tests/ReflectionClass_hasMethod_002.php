<?php
class C {
	function f() {}
}

$rc = new ReflectionClass("C");
echo "Check invalid params:\n";
var_dump($rc->hasMethod());
var_dump($rc->hasMethod("f", "f"));
var_dump($rc->hasMethod(null));
var_dump($rc->hasMethod(1));
var_dump($rc->hasMethod(1.5));
var_dump($rc->hasMethod(true));
var_dump($rc->hasMethod(array(1,2,3)));
var_dump($rc->hasMethod(new C));
?>
