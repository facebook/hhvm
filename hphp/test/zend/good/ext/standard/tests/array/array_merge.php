<?hh
/* Prototype: array array_merge(array $array1 [, array $array2 [, array $...]]);
   Description: Merge one or more arrays
*/
<<__EntryPoint>> function main(): void {
echo "\n*** Testing array_merge() basic functionality ***";
$begin_array = vec[
  vec[],
  dict[ 1 => "string"],
  dict[ "" => "string"],
  dict[ -2 => 12],
  dict[ "a" => 1, "b" => -2.344, "b" => "string", "c" => NULL,    "d" => -2.344],
  dict[ 4 => 1, 3 => -2.344, "3" => "string", "2" => NULL,1 => -2.344],
  dict[ 0 => NULL, 1 => "Hi", "string" => "hello",
          2 => dict["" => "World", "-2.34" => "a", "0" => "b"]]
];

$end_array   = vec[
  vec[],
  dict[ 1 => "string"],
  dict[ "" => "string"],
  dict[ -2 => 12],
  dict[ "a" => 1, "b" => -2.344, "b" => "string", "c" => NULL, "d" => -2.344],
  dict[ 4 => 1, 3 => -2.344, "3" => "string", "2" => NULL, 1=> -2.344],
  dict[ 0 => NULL, 1 => "Hi", "string" => "hello",
          2 => dict["" => "World", "-2.34" => "a", "0" => "b"]]
];

/* loop through to merge two arrays */
$count_outer = 0;
foreach($begin_array as $first) {
  echo "\n\n--- Iteration $count_outer ---";
  $count_inner = 0;
  foreach($end_array as $second) {
    echo "\n-- Inner iteration $count_inner of Iteration $count_outer --\n";
    $result = array_merge($first, $second);
    print_r($result);
    $count_inner++;
  }
  $count_outer++;
}


echo "\n*** Testing array_merge() with three or more arrays ***\n";
var_dump( array_merge( $end_array[0],
                       $end_array[5],
                       $end_array[4],
                       $end_array[6]
                     )
        );

var_dump( array_merge( $end_array[0],
                       $end_array[5],
                       vec["array on fly"],
                       dict["nullarray" => vec[]]
                     )
        );


echo "\n*** Testing single array argument ***\n";
/* Empty array */
var_dump(array_merge(vec[]));

/* associative array with string keys, which will not be re-indexed */
var_dump(array_merge($begin_array[4]));

/* associative array with numeric keys, which will be re-indexed */
var_dump(array_merge($begin_array[5]));

/* associative array with mixed keys and sub-array as element */
var_dump(array_merge($begin_array[6]));

echo "\n*** Testing error conditions ***";
/* Invalid arguments */
try { var_dump(array_merge()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(array_merge(100, 200));
var_dump(array_merge($begin_array[0], $begin_array[1], 100));
try {
  var_dump(array_merge($begin_array[0], $begin_array[1], $arr4));
} catch (UndefinedVariableException $e) {
  var_dump($e->getMessage());
}

echo "Done\n";
}
