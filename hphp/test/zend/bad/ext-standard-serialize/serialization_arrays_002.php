<?php
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

function check(&$a) {
	var_dump($a);
	$ser = serialize($a);
	var_dump($ser);
	
	$b = unserialize($ser);
	var_dump($b);
	$b[0] = "b0.changed";
	var_dump($b);
	$b[1] = "b1.changed";
	var_dump($b);
	$b[2] = "b2.changed";
	var_dump($b);	
}

echo "\n\n--- No references:\n";
$a = array();
$a[0] = 1;
$a[1] = 1;
$a[2] = 1;
check($a);

echo "\n\n--- 0 refs 1:\n";
$a = array();
$a[0] = &$a[1];
$a[1] = 1;
$a[2] = 1;
check($a);

echo "\n\n--- 0 refs 2:\n";
$a = array();
$a[0] = &$a[2];
$a[1] = 1;
$a[2] = 1;
check($a);

echo "\n\n--- 1 refs 0:\n";
$a = array();
$a[0] = 1;
$a[1] = &$a[0];
$a[2] = 1;
check($a);

echo "\n\n--- 1 refs 2:\n";
$a = array();
$a[0] = 1;
$a[1] = &$a[2];
$a[2] = 1;
check($a);

echo "\n\n--- 2 refs 0:\n";
$a = array();
$a[0] = 1;
$a[1] = 1;
$a[2] = &$a[0];
check($a);

echo "\n\n--- 2 refs 1:\n";
$a = array();
$a[0] = 1;
$a[1] = 1;
$a[2] = &$a[1];
check($a);

echo "\n\n--- 0,1 ref 2:\n";
$a = array();
$a[0] = &$a[2];
$a[1] = &$a[2];
$a[2] = 1;
check($a);

echo "\n\n--- 0,2 ref 1:\n";
$a = array();
$a[0] = &$a[1];
$a[1] = 1;
$a[2] = &$a[1];
check($a);

echo "\n\n--- 1,2 ref 0:\n";
$a = array();
$a[0] = 1;
$a[1] = &$a[0];
$a[2] = &$a[0];
check($a);

echo "Done";
?>