<?hh
/* Prototype  : array array_intersect(array $arr1, array $arr2 [, array $...])
 * Description: Returns the entries of arr1 that have values which are present in all the other arguments
 * Source code: ext/standard/array.c
*/

/*
 * Testing the functionality of array_intersect() by passing different
 * associative arrays having different possible values to $arr2 argument.
 * The $arr1 argument is a fixed array.
*/

// get a class
class classA
{
  public function __toString():mixed{
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_intersect() : assoc array with diff values to \$arr2 argument ***\n";


// get a resource variable
$fp = fopen(__FILE__, "r");

// get a heredoc string
$heredoc = <<<EOT
Hello world
EOT;

// different variations of associative arrays to be passed to $arr2 argument
$arrays = vec[

       // empty array
/*1*/  vec[],

       // arrays with integer values
       dict['0' => 0],
       dict["1" => 1],
       dict["one" => 1, 'two' => 2, "three" => 3, 4 => 4],

       // arrays with float values
/*5*/  dict["float" => 2.3333],
       dict["f1" => 1.2, 'f2' => 3.33, 3 => 4.89999922839999, 'f4' => 33333333.333333],

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

// array to be passsed to $arr1 argument
$arr1 = vec[1, 2, 1.2, 2.3333, "col\tor", '\v\fworld', $fp,
              "Hello world", $heredoc, new classA(), 444.432, "fruit"];

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

// close the file resource used
fclose($fp);

echo "Done";
}
