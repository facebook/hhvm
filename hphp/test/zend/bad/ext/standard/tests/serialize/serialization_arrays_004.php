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

echo "\n\n--- 1 refs container:\n";
$a = array();
$a[0] = &$a;
$a[1] = 1;
$a[2] = 1;
check($a);

echo "\n\n--- 1,2 ref container:\n";
$a = array();
$a[0] = &$a;
$a[1] = &$a;
$a[2] = 1;
check($a);

echo "\n\n--- 1,2,3 ref container:\n";
$a = array();
$a[0] = &$a;
$a[1] = &$a;
$a[2] = &$a;
check($a);

echo "Done";
?>