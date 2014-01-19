<?php

class C {
	public $a = "Original a";
	public $b = "Original b";
	public $c = "Original c";
	public $d = "Original d";
	public $e = "Original e";
}

echo "\nRemoving properties before the current element from an iterated object.\n";
$obj = new C;
$count=0;
foreach ($obj as $v) {
	if ($v==$obj->a) {
		unset($obj->c);	
	}
	var_dump($v);
	if (++$count>10) {
		echo "Loop detected.\n";
		break;
	}		
}
var_dump($obj);

echo "\nRemoving properties before the current element from an iterated object.\n";
$obj = new C;
foreach ($obj as $v) {
	if ($v==$obj->b) {
		unset($obj->a);	
	}
	var_dump($v);
	if (++$count>10) {
		echo "Loop detected.\n";
		break;
	}	
}
var_dump($obj);


?>