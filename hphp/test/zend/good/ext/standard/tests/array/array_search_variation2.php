<?hh
/*
 * Prototype  : mixed array_search ( mixed $needle, array $haystack [, bool $strict] )
 * Description: Searches haystack for needle and returns the key if it is found in the array, FALSE otherwise
 * Source Code: ext/standard/array.c
*/

/* Test array_search() with different possible haystack values */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_search() with different haystack values ***\n";

$misc_array = dict[
  0 => 'a',
  'key' =>'d',
  1 => 3,
  ".001" =>-67,
  "-.051" =>"k",
  0 =>"-.08",
  "e" =>"5",
  "y" =>NULL,
  '' =>"",
  2 => 0,
  3 => TRUE,
  4 => FALSE,
  5 => -27.39999999999,
  6 => " ",
  7 => "abcd\x00abcd\x00\abcd\x00abcdefghij",
  8 => "abcd\nabcd\tabcd\rabcd\0abcd"
];
$array_type = vec[TRUE, FALSE, 1, 0, -1, "1", "0", "-1", NULL, vec[], "PHP", ""];
/* loop to do loose and strict type check of elements in
   $array_type on elements in $misc_array using array_search();
   checking PHP type comparison tables
*/
$counter = 1;
foreach($array_type as $type) {
  echo "-- Iteration $counter --\n";
  //loose type checking
  var_dump( array_search($type,$misc_array ) );
  //strict type checking
  var_dump( array_search($type,$misc_array,true) );
  //loose type checking
  var_dump( array_search($type,$misc_array,false) );
  $counter++;
}

echo "Done\n";
}
