<?hh
/*
  Prototype: mixed str_replace(mixed $search, mixed $replace,
                               mixed $subject [, int &$count]);
  Description: Replace all occurrences of the search string with
               the replacement string
*/

<<__EntryPoint>> function main(): void {
echo "\n*** Testing str_replace() with various search values ***";
$search_arr = vec[ TRUE, FALSE, 1, 0, -1, "1", "0", "-1",  NULL,
                     vec[], "php", ""];

$i = 0;
/* loop through to replace the matched elements in the array */
foreach( $search_arr as $value ) {
  echo "\n-- Iteration $i --\n";
  /* replace the string in array */
  $count = 0;
  var_dump( str_replace_with_count($value, "FOUND", $search_arr, inout $count) );
  var_dump( $count );
  $i++;
}

echo "===DONE===\n";
}
