<?hh
/* Prototype  : array array_intersect(array $arr1, array $arr2 [, array $...])
 * Description: Returns the entries of arr1 that have values which are present in all the other arguments
 * Source code: ext/standard/array.c
*/

/*
 * Testing the functionality of array_intersect() by passing different
 * associative arrays having different possible keys to $arr2 argument.
 * The $arr1 argument is a fixed array.
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_intersect() : assoc array with diff keys to \$arr2 argument ***\n";

// get a heredoc string
$heredoc = <<<EOT
Hello world
EOT;

// different variations of associative arrays to be passed to $arr2 argument
$arrays = vec[

       // empty array
/*1*/  vec[],

       // arrays with integer keys
       dict[0 => "0"],
       dict[1 => "1"],
       dict[1 => "1", 2 => "2", 3 => "3", 4 => "4"],

       // arrays with float keys
/*5*/  dict[2 => "float"],
       dict[1 => "f1", 3 => "f2",
             4 => "f3",
             33333333 => "f4"],

       // arrays with string keys
/*7*/  dict['\tHello' => 111, 're\td' => "color",
             '\v\fworld' => 2.2, 'pen\n' => 33],
       dict["\tHello" => 111, "re\td" => "color",
             "\v\fworld" => 2.2, "pen\n" => 33],
       dict[0 => "hello", $heredoc => "string"], // heredoc

       // array with unset variable
/*10*/ dict[ '' => "hello"],

       // array with mixed keys
/*11*/ dict['hello' => 1,  "fruit" => 2.2,
             133 => "int", 444 => "float",
             '' => "unset", $heredoc => "heredoc"]
];

// array to be passsed to $arr1 argument
$arr1 = vec[1, "float", "f4", "hello", 2.2, 'color', "string", "pen\n", 11];

// loop through each sub-array within $arrrays to check the behavior of array_intersect()
$iterator = 1;
foreach($arrays as $arr2) {
  echo "-- Iterator $iterator --\n";

  // Calling array_intersect() with default arguments
  var_dump( array_intersect($arr1, $arr2) );

  // Calling array_intersect() with more arguments.
  // additional argument passed is the same as $arr1 argument
  var_dump( array_intersect($arr1, $arr2, $arr1) );
  $iterator++;
}

echo "Done";
}
