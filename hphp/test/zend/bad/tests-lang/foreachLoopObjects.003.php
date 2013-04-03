<?php

class C {
	public $a = "Original a";
	public $b = "Original b";
	public $c = "Original c";
	protected $d = "Original d";
	private $e = "Original e";
}

echo "\nDirectly changing object values.\n";
$obj = new C;
foreach ($obj as $k=>$v) {
	$obj->$k="changed.$k";
	var_dump($v);
}
var_dump($obj);

echo "\nModifying the foreach \$value.\n";
$obj = new C;
foreach ($obj as $k=>$v) {
	$v="changed.$k";
}
var_dump($obj);


echo "\nModifying the foreach &\$value.\n";
$obj = new C;
foreach ($obj as $k=>&$v) {
	$v="changed.$k";
}
var_dump($obj);

echo "\nAdding properties to an an object.\n";
$obj = new C;
$counter=0;
foreach ($obj as $v) {
	$newPropName = "new$counter";
	$obj->$newPropName = "Added property $counter";
    if ($counter++>10) {
    	echo "Loop detected\n";
    	break;
    }
	var_dump($v);    
}
var_dump($obj);

echo "\nAdding properties to an an object, using &\$value.\n";
$obj = new C;
$counter=0;
foreach ($obj as &$v) {
	$newPropName = "new$counter";
	$obj->$newPropName = "Added property $counter";
    if ($counter++>10) {
    	echo "Loop detected\n";
    	break;    	
    }
	var_dump($v);    
}
var_dump($obj);

echo "\nRemoving properties from an object.\n";
$obj = new C;
foreach ($obj as $v) {
	unset($obj->a);
	unset($obj->b);
	unset($obj->c);	
	var_dump($v);
}
var_dump($obj);

echo "\nRemoving properties from an object, using &\$value.\n";
$obj = new C;
foreach ($obj as &$v) {
	unset($obj->a);
	unset($obj->b);
	unset($obj->c);
	var_dump($v);
}
var_dump($obj);

?>
===DONE===