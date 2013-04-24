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
	
	// Change each element and dump result. 
	foreach($b as $k=>$v) {
		if (is_array($v)){
			foreach($b[$k] as $sk=>$sv) {
				$b[$k][$sk] = "b$k.$sk.changed";
				var_dump($b);
			}
		} else {
			$b[$k] = "b$k.changed";
			var_dump($b);
		}
	}
}

echo "\n\n--- Nested array references 1 element in containing array:\n";
$a = array();
$c = array(1,1,&$a);
$a[0] = &$c[0];
$a[1] = 1;
check($c);

echo "\n\n--- Nested array references 1 element in containing array (slightly different):\n";
$a = array();
$c = array(1,&$a,1);
$a[0] = 1;
$a[1] = &$c[0];
check($c);

echo "\n\n--- Nested array references 2 elements in containing array:\n";
$a = array();
$c = array(1,1,&$a);
$a[0] = &$c[0];
$a[1] = &$c[1];
check($c);


echo "\n\n--- Containing array references 1 element in nested array:\n";
$a = array();
$a[0] = 1;
$a[1] = 1;
$c = array(1,&$a[0],&$a);
check($c);

echo "\n\n--- Containing array references 2 elements in nested array:\n";
$a = array();
$a[0] = 1;
$a[1] = 1;
$c = array(&$a[0],&$a[1],&$a);
check($c);

echo "\n\n--- Nested array references container:\n";
$a = array();
$c = array(1,1,&$a);
$a[0] = 1;
$a[1] = &$c;
check($c);

?>