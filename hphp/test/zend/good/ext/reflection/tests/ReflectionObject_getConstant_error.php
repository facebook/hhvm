<?php
class C {
	const myConst = 1;
}

$rc = new ReflectionObject(new C);
var_dump($rc->getConstant());
var_dump($rc->getConstant("myConst", "myConst"));
var_dump($rc->getConstant(null));
var_dump($rc->getConstant(1));
var_dump($rc->getConstant(1.5));
var_dump($rc->getConstant(true));
var_dump($rc->getConstant(array(1,2,3)));
var_dump($rc->getConstant(new C));
?>
