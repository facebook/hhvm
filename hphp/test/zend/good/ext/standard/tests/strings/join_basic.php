<?hh
/* Prototype  : string join( string $glue, array $pieces )
 * Description: Join array elements with a string
 * Source code: ext/standard/string.c
 * Alias of function: implode()
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing join() : basic functionality ***\n";

// Initialize all required variables
$glue = ',';
$pieces = vec[1, 2, 3, 4];

// pieces as arry with numeric values
var_dump( join($glue, $pieces) );

// pieces as array with strings values
$glue = ", "; // multiple car as glue
$pieces = vec["Red", "Green", "Blue", "Black", "White"];
var_dump( join($glue, $pieces) );

// pices as associative array (numeric values)
$pieces = dict["Hour" => 10, "Minute" => 20, "Second" => 40];
$glue = ':';
var_dump( join($glue, $pieces) );

// pices as associative array (string/numeric values)
$pieces = dict["Day" => 'Friday', "Month" => "September", "Year" => 2007];
$glue = '/';
var_dump( join($glue, $pieces) );

echo "Done\n";
}
