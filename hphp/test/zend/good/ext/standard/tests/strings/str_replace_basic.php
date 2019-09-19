<?hh
/*
  Prototype: mixed str_replace(mixed $search, mixed $replace,
                               mixed $subject [, int &$count]);
  Description: Replace all occurrences of the search string with
               the replacement string
*/
<<__EntryPoint>> function main(): void {
echo "\n*** Testing str_replace() on basic operations ***\n";
$count = 0;
var_dump( str_replace("", "", "") );

var_dump( str_replace("e", "b", "test") );
var_dump( str_replace_with_count("", "", "", inout $count) );
var_dump( $count );

var_dump( str_replace_with_count("q", "q", "q", inout $count) );
var_dump( $count );

var_dump( str_replace_with_count("long string here", "", "", inout $count) );
var_dump( $count );

$fp = fopen( __FILE__, "r" );
$fp_copy = $fp;
var_dump( str_replace_with_count($fp_copy, $fp_copy, $fp_copy, inout $fp_copy) );
var_dump( $fp_copy );
fclose($fp);

echo "===DONE===\n";
}
