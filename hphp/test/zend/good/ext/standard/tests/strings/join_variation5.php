<?hh
/* Prototype  : string join( string $glue, array $pieces )
 * Description: Join array elements with a string
 * Source code: ext/standard/string.c
 * Alias of function: implode()
*/

/*
 * test join() by passing pieces as array containing sub array(s)
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing implode() : usage variations - sub arrays ***\n";
$sub_array = varray[varray[1,2,3,4], darray[1 => "one", 2 => "two"], "PHP", 50];

// pieces as array containing sub array
var_dump( join("TEST", $sub_array) );

// glue as array & pieces as array containing sub array
var_dump( join(varray[1, 2, 3, 4], $sub_array) );

// numeric value as glue, pieces as array containg sub array
var_dump( join(2, $sub_array) );

// using directly the sub_array as pieces
var_dump( join(", ", $sub_array[0]) );
var_dump( join(", ", $sub_array[1]) );

echo "Done\n";
}
