<?hh
/* Prototype  : array array_intersect_assoc(array $arr1, array $arr2 [, array $...])
 * Description: Returns the entries of arr1 that have values which are present in all the other arguments.
 * Keys are used to do more restrictive check
 * Source code: ext/standard/array.c
*/

/*
 * Testing the functionality of array_intersect_assoc() by passing different
 * associative arrays having different possible keys to $arr1 argument.
 * The $arr2 argument passed is a fixed array
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_intersect_assoc() : assoc array with diff keys to \$arr1 argument ***\n";

// get a heredoc string
$heredoc = <<<EOT
Hello world
EOT;

// different variations of associative arrays to be passed to $arr1 argument
$arrays = varray [

       // empty array
/*1*/  varray[],

       // arrays with integer keys
       darray[0 => "0"],
       darray[1 => "1"],
       darray[1 => "1", 2 => "2", 3 => "3", 4 => "4"],

       // arrays with float keys
/*5*/  darray[2 => "float"],
       darray[1 => "f1", 3 => "f2",
             4 => "f3",
             33333333 => "f4"],

       // arrays with string keys
/*7*/  darray['\tHello' => 111, 're\td' => "color",
             '\v\fworld' => 2.2, 'pen\n' => 33],
       darray["\tHello" => 111, "re\td" => "color",
             "\v\fworld" => 2.2, "pen\n" => 33],
       darray[0 => "hello", $heredoc => "string"], // heredoc

       // array with object, unset variable and resource variable
/*10*/ darray['' => "hello"],

       // array with mixed keys
/*11*/ darray['hello' => 1, "fruit" => 2.2,
              133 => "int", 444 => "float",
             '' => "unset", $heredoc => "heredoc"]
];

// array to be passsed to $arr2 argument
$arr2 = darray[0 => 0, 2 => "float", 4 => "f3", 33333333 => "f4",
              "\tHello" => 111, 33333334 => 2.2, 33333335 => 'color', "Hello world" => "string",
              "pen\n" => 33,  133 => "int"];

// loop through each sub-array within $arrrays to check the behavior of array_intersect_assoc()
$iterator = 1;
foreach($arrays as $arr1) {
  echo "-- Iteration $iterator --\n";

  // Calling array_intersect_assoc() with default arguments
  var_dump( array_intersect_assoc($arr1, $arr2) );

  // Calling array_intersect_assoc() with more arguments.
  // additional argument passed is the same as $arr1 argument
  var_dump( array_intersect_assoc($arr1, $arr2, $arr1) );
  $iterator++;
}

echo "Done";
}
