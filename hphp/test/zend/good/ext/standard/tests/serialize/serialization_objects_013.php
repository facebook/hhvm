<?php
ini_set('error_reporting ',  E_ALL & ~E_STRICT);

/* Prototype  : proto string serialize(mixed variable)
 * Description: Returns a string representation of variable (which can later be unserialized) 
 * Source code: ext/standard/var.c
 * Alias to functions: 
 */
/* Prototype  : proto mixed unserialize(string variable_representation)
 * Description: Takes a string representation of variable and recreates it 
 * Source code: ext/standard/var.c
 * Alias to functions: 
 */

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

echo "\n\n--- a refs b:\n";
$obj = new stdClass;
$obj->a = &$obj->b;
$obj->b = 1;
$obj->c = 1;
check($obj);

echo "\n\n--- a refs c:\n";
$obj = new stdClass;
$obj->a = &$obj->c;
$obj->b = 1;
$obj->c = 1;
check($obj);

echo "\n\n--- b refs a:\n";
$obj = new stdClass;
$obj->a = 1;
$obj->b = &$obj->a;
$obj->c = 1;
check($obj);

echo "\n\n--- b refs c:\n";
$obj = new stdClass;
$obj->a = 1;
$obj->b = &$obj->c;
$obj->c = 1;
check($obj);

echo "\n\n--- c refs a:\n";
$obj = new stdClass;
$obj->a = 1;
$obj->b = 1;
$obj->c = &$obj->a;
check($obj);

echo "\n\n--- c refs b:\n";
$obj = new stdClass;
$obj->a = 1;
$obj->b = 1;
$obj->c = &$obj->b;
check($obj);

echo "\n\n--- a,b refs c:\n";
$obj = new stdClass;
$obj->a = &$obj->c;
$obj->b = &$obj->c;
$obj->c = 1;
check($obj);

echo "\n\n--- a,c refs b:\n";
$obj = new stdClass;
$obj->a = &$obj->b;
$obj->b = 1;
$obj->c = &$obj->b;
check($obj);

echo "\n\n--- b,c refs a:\n";
$obj = new stdClass;
$obj->a = 1;
$obj->b = &$obj->a;
$obj->c = &$obj->a;
check($obj);

echo "Done";
?>