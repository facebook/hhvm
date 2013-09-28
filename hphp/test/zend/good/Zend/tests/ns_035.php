<?php
namespace A;
use \ArrayObject;

function f1($x = ArrayObject::STD_PROP_LIST) {
	var_dump($x);
}
function f2($x = \ArrayObject::STD_PROP_LIST) {
	var_dump($x);
}
var_dump(ArrayObject::STD_PROP_LIST);
var_dump(\ArrayObject::STD_PROP_LIST);
f1();
f2();

?>