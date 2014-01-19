<?php
/* Prototype  : bool array_key_exists(mixed $key, array $search)
 * Description: Checks if the given key or index exists in the array 
 * Source code: ext/standard/array.c
 * Alias to functions: key_exists
 */

/*
 * Pass certain data types that can be taken as a key in an array 
 * and test whether array_key_exists(() thinks they are equal and therefore
 * returns true when searching for them
 */

echo "*** Testing array_key_exists() : usage variations ***\n";

$unset = 10;
unset($unset);
$array = array ('null' => null, 
                'NULL' => NULL, 
                'empty single quoted string' => '', 
                "empty double quoted string" => "", 
                'undefined variable' => @$undefined,
                'unset variable' => @$unset);

//iterate through original array
foreach($array as $name => $input) {
	$iterator = 1;
	echo "\n-- Key in \$search array is : $name --\n";
	$search[$input] = 'test';
	
	//iterate through array again to see which values are considered equal
	foreach($array as $key) {
		echo "Iteration $iterator:  ";
		var_dump(array_key_exists($key, $search));
		$iterator++;
	}
	$search = null;
}

echo "Done";
?>
