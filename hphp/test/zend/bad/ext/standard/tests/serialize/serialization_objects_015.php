<?php
ini_set('error_reporting ',  E_ALL & ~E_STRICT);


function check(&$obj) {
	var_dump($obj);
	$ser = serialize($obj);
	var_dump($ser);
	
	$uobj = unserialize($ser);
	var_dump($uobj);
	$uobj->a = "obj->a.changed";
	var_dump($uobj);
	$uobj->b = "obj->b.changed";
	var_dump($uobj);
	$uobj->c = "obj->c.changed";
	var_dump($uobj);	
}

echo "\n\n--- a refs container:\n";
$ext = 1;
$obj = new stdClass;
$obj->a = &$obj;
$obj->b = 1;
$obj->c = 1;
check($obj);

echo "\n\n--- a eqs container:\n";
$ext = 1;
$obj = new stdClass;
$obj->a = $obj;
$obj->b = 1;
$obj->c = 1;
check($obj);

echo "\n\n--- a,b ref container:\n";
$ext = 1;
$obj = new stdClass;
$obj->a = &$obj;
$obj->b = &$obj;
$obj->c = 1;
check($obj);

echo "\n\n--- a,b eq container:\n";
$ext = 1;
$obj = new stdClass;
$obj->a = $obj;
$obj->b = $obj;
$obj->c = 1;
check($obj);

echo "\n\n--- a,b,c ref container:\n";
$ext = 1;
$obj = new stdClass;
$obj->a = &$obj;
$obj->b = &$obj;
$obj->c = &$obj;
check($obj);

echo "\n\n--- a,b,c eq container:\n";
$ext = 1;
$obj = new stdClass;
$obj->a = $obj;
$obj->b = $obj;
$obj->c = $obj;
check($obj);

echo "Done";
?>