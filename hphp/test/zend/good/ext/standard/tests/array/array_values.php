<?hh
/* Prototype: array array_values ( array $input );
   Discription: array_values() returns all the values from the input array 
                and indexes numerically the array
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_values() on basic array ***\n"; 
$basic_arr = varray[ 1, 2, 2.0, "asdasd", varray[1,2,3] ];
var_dump( array_values($basic_arr) );

echo "\n*** Testing array_values() on various arrays ***";
$arrays = varray [
  varray[], 
  varray[0],
  varray[-1],
  varray[ varray[] ],
  varray["Hello"],
  varray[""],  
  varray["", varray[]],
  varray[1,2,3], 
  varray[1,2,3, varray[]],
  varray[1,2,3, varray[4,6]],
  darray["a" => 1, "b" => 2, "c" =>3],
  darray[0 => 0, 1 => 1, 2 => 2],  
  varray[TRUE, FALSE, NULL, true, false, null, "TRUE", "FALSE",
        "NULL", "\x000", "\000"],
  darray["Hi" => 1, "Hello" => 2, "World" => 3],
  darray["a" => "abcd", "a" => "", "ab" => -6, "cd" => -0.5 ],
  darray[0 => varray[], 1=> varray[0], 2 => varray[1], ""=> varray[], ""=>"" ]
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
