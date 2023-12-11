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
$sub_array = vec[vec[1,2,3,4], dict[1 => "one", 2 => "two"], "PHP", 50];

// using directly the sub_array as pieces
var_dump( join(", ", $sub_array[0]) );
var_dump( join(", ", $sub_array[1]) );

echo "Done\n";
}
