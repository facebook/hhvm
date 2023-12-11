<?hh
/* Prototype: array array_values ( array $input );
   Discription: array_values() returns all the values from the input array 
                and indexes numerically the array
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_values() on basic array ***\n"; 
$basic_arr = vec[ 1, 2, 2.0, "asdasd", vec[1,2,3] ];
var_dump( array_values($basic_arr) );

echo "\n*** Testing array_values() on various arrays ***";
$arrays = varray [
  vec[], 
  vec[0],
  vec[-1],
  vec[ vec[] ],
  vec["Hello"],
  vec[""],  
  vec["", vec[]],
  vec[1,2,3], 
  vec[1,2,3, vec[]],
  vec[1,2,3, vec[4,6]],
  dict["a" => 1, "b" => 2, "c" =>3],
  dict[0 => 0, 1 => 1, 2 => 2],  
  vec[TRUE, FALSE, NULL, true, false, null, "TRUE", "FALSE",
        "NULL", "\x000", "\000"],
  dict["Hi" => 1, "Hello" => 2, "World" => 3],
  dict["a" => "abcd", "a" => "", "ab" => -6, "cd" => -0.5 ],
  dict[0 => vec[], 1=> vec[0], 2 => vec[1], ""=> vec[], ""=>"" ]
];

$i = 0;
/* loop through to test array_values() with different arrays given above */
foreach ($arrays as $array) {
  echo "\n-- Iteration $i --\n";
  var_dump( array_values($array) );
  $i++;
}

echo "Done\n";
}
