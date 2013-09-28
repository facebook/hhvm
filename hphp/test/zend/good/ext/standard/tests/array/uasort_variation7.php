<?php
/* Prototype  : bool uasort(array $array_arg, string $cmp_function)
 * Description: Sort an array with a user-defined comparison function and maintain index association 
 * Source code: ext/standard/array.c
*/

/*
* Passing different anonymous functions as 'cmp_function'
*   arguments passed by value
*   arguments passed by reference
*/

echo "*** Testing uasort() : anonymous function as 'cmp_function' ***\n";

$cmp_function = 'if($value1 == $value2) {return 0;} else if($value1 > $value2) {return 1;} else{return -1;}';

$array_arg = array(0 => 100, 1 => 3, 2 => -70, 3 => 24, 4 => 90);
echo "-- Anonymous 'cmp_function' with parameters passed by value --\n";
var_dump( uasort($array_arg, create_function('$value1, $value2',$cmp_function) ) );
var_dump($array_arg);

$array_arg = array("b" => "Banana", "m" => "Mango", "a" => "Apple", "p" => "Pineapple");
echo "-- Anonymous 'cmp_function' with parameters passed by reference --\n";
var_dump( uasort($array_arg, create_function('&$value1, &$value2', $cmp_function) ) );
var_dump($array_arg);

echo "Done"
?>