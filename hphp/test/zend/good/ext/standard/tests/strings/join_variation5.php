<?php
/* Prototype  : string join( string $glue, array $pieces )
 * Description: Join array elements with a string
 * Source code: ext/standard/string.c
 * Alias of function: implode()
*/

/*
 * test join() by passing pieces as array containing sub array(s)
*/

echo "*** Testing implode() : usage variations - sub arrays ***\n";
$sub_array = array(array(1,2,3,4), array(1 => "one", 2 => "two"), "PHP", 50);

// pieces as array containing sub array
var_dump( join("TEST", $sub_array) );

// glue as array & pieces as array containing sub array
var_dump( join(array(1, 2, 3, 4), $sub_array) );

// numeric value as glue, pieces as array containg sub array
var_dump( join(2, $sub_array) );

// using directly the sub_array as pieces
var_dump( join(", ", $sub_array[0]) );
var_dump( join(", ", $sub_array[1]) );

echo "Done\n";
?>