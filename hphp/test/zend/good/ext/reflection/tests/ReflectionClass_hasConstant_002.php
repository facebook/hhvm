<?php
class C {
	const myConst = 1;
}

$rc = new ReflectionClass("C");
echo "Check invalid params:\n";
var_dump($rc->hasConstant());
var_dump($rc->hasConstant("myConst", "myConst"));
var_dump($rc->hasConstant(null));
var_dump($rc->hasConstant(1));
var_dump($rc->hasConstant(1.5));
var_dump($rc->hasConstant(true));
var_dump($rc->hasConstant(array(1,2,3)));
var_dump($rc->hasConstant(new C));
?>
