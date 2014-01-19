<?php

class C {
	public $a = "Original a";
	public $b = "Original b";
	public $c = "Original c";
	public $d = "Original d";
	public $e = "Original e";
}

echo "\nSubstituting the iterated object for a different object.\n";
$obj = new C;
$obj2 = new stdclass;
$obj2->a = "new a";
$obj2->b = "new b";
$obj2->c = "new c";
$obj2->d = "new d";
$obj2->e = "new e";
$obj2->f = "new f";
$ref = &$obj;
$count=0;
foreach ($obj as $v) {
	var_dump($v);
	if ($v==$obj->b) {
	  $ref=$obj2;
	}
	if (++$count>10) {
		echo "Loop detected.\n";
		break;
	}	
}
var_dump($obj);

echo "\nSubstituting the iterated object for an array.\n";
$obj = new C;
$a = array(1,2,3,4,5,6,7,8);
$ref = &$obj;
$count=0;
foreach ($obj as $v) {
	var_dump($v);
	if ($v==="Original b") {
	  $ref=$a;
	}
	if (++$count>10) {
		echo "Loop detected.\n";
		break;
	}	
}
var_dump($obj);

echo "\nSubstituting the iterated array for an object.\n";
$a = array(1,2,3,4,5,6,7,8);
$obj = new C;
$ref = &$a;
$count=0;
foreach ($a as $v) {
	var_dump($v);
	if ($v===2) {
	  $ref=$obj;
	}
	if (++$count>10) {
		echo "Loop detected.\n";
		break;
	}
}
var_dump($obj);

?>