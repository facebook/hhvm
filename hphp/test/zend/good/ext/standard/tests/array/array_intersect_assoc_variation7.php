<?hh
/* Prototype  : array array_intersect_assoc(array $arr1, array $arr2 [, array $...])
 * Description: Returns the entries of arr1 that have values which are present in all the other arguments.
 * Keys are used to do more restrictive check
 * Source code: ext/standard/array.c
*/

/*
 * Testing the functionality of array_intersect_assoc() by passing different
 * associative arrays having different possible values to $arr1 argument.
 * The $arr2 argument passed is a fixed array
*/

// get a class
class classA
{
  public function __toString():mixed{
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_intersect_assoc() : assoc array with diff values to \$arr1 argument ***\n";


// get a resource variable
$fp = fopen(__FILE__, "r");

// get a heredoc string
$heredoc = <<<EOT
Hello world
EOT;

// different variations of associative arrays to be passed to $arr1 argument
$arrays = vec[

       // empty array
/*1*/  vec[],

       // arrays with integer values
       dict['0' => 0],
       dict["1" => 1],
       dict["one" => 1, 'two' => 2, "three" => 3, 4 => 4],

       // arrays with float values
/*5*/  dict["float" => 2.3333],
       dict["f1" => 1.2, 'f2' => 3.33, 3 => 4.89999922839999, 'f4' => 33333333.333],

       // arrays with string values
/*7*/  dict[111 => "\tHello", "red" => "col\tor", 2 => "\v\fworld", 3 =>  "pen\n"],
       dict[111 => '\tHello', "red" => 'col\tor', 2 => '\v\fworld', 3 =>  'pen\n'],
       dict[1 => "hello", "heredoc" => $heredoc],

       // array with object, unset variable and resource variable
/*10*/ dict[11 => new classA(), "resource" => $fp],

       // array with mixed values
/*11*/ dict[1 => 'hello', 2 => new classA(), 222 => "fruit",
             'resource' => $fp, "int" => 133, "float" => 444.432,
             "heredoc" => $heredoc]
];

// array to be passsed to $arr2 argument
$arr2 = dict[0 => "0", 1 => 1, "two" => 2, "float" => 2.3333, "f1" => 1.2,
              "f4" => 33333333.333, 111 => "\tHello", 3 => 'pen\n', 33333334 => '\v\fworld',
              "heredoc" => "Hello world", 11 => new classA(), "resource" => $fp,
              "int" => 133, 222 => "fruit"];

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

// close the file resource used
fclose($fp);

echo "Done";
}
