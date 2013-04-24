<?php
/* Prototype  : array array_change_key_case(array $input [, int $case])
 * Description: Retuns an array with all string keys lowercased [or uppercased] 
 * Source code: ext/standard/array.c
 */

/*
 * Pass different integer values as $case argument to array_change_key_case() to test behaviour
 */

echo "*** Testing array_change_key_case() : usage variations ***\n";

$input = array('One' => 'un', 'TWO' => 'deux', 'three' => 'trois');
for ($i = -5; $i <=5; $i += 1){
	echo "\n-- \$sort argument is $i --\n";
	$temp = $input;
	var_dump(array_change_key_case($temp, $i));
}

echo "Done";
?>
