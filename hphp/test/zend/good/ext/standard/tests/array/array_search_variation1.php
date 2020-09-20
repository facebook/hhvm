<?hh
/*
 * Prototype  : mixed array_search ( mixed $needle, array $haystack [, bool $strict] )
 * Description: Searches haystack for needle and returns the key if it is found in the array, FALSE otherwise
 * Source Code: ext/standard/array.c
*/

<<__EntryPoint>> function main(): void {
echo "*** Testing array_search() with different needle values ***\n";
$arrays = varray [
  varray[0],
  darray["a" => "A", 2 => "B", "C" => 3, 4 => 4, "one" => 1, "" => NULL, 5 => "b", 6 => "ab", 7 => "abcd"],
  darray[0 => 4, 1 => darray[0 => 1, 2 => 3], "one" => 1, "5" => 5],
  darray[0 => -1, 1 => -2, 2 => -3, 3 => -4, 4 => -2.989888, "-0.005" => "neg0.005", 2 => "float2", "-.9" => -.9],
  varray[TRUE, FALSE],
  varray["", varray[]],
  varray["abcd\x00abcd\x00abcd"],
  varray["abcd\tabcd\nabcd\rabcd\0abcdefghij"]
];

$array_compare = varray [
  4,
  "4",
  4.00,
  "b",
  "5",
  -2,
  -2.0,
  -2.98989,
  "-.9",
  "True",
  "",
  varray[],
  NULL,
  "ab",
  "abcd",
  0.0,
  -0,
  "abcd\x00abcd\x00abcd"
];
/* loop to check if elements in $array_compare exist in $arrays
   using array_search() */
$counter = 1;
foreach($arrays as $array) {
  foreach($array_compare as $compare) {
    echo "-- Iteration $counter --\n";
    //strict option OFF
    var_dump(array_search($compare,$array));
    //strict option ON
    var_dump(array_search($compare,$array,TRUE));
    //strict option OFF
    var_dump(array_search($compare,$array,FALSE));
    $counter++;
  }
}

echo "Done\n";
}
