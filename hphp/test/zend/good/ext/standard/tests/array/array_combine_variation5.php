<?hh
/* Prototype  : array array_combine(array $keys, array $values)
 * Description: Creates an array by using the elements of the first parameter as keys
 *              and the elements of the second as the corresponding values
 * Source code: ext/standard/array.c
*/

/*
* Testing the functionality of array_combine() by passing various
* associative arrays having different possible values to $keys argument and
* associative arrays having different possible values to $values argument.
*/

// get a class
class classA
{
  public function __toString():mixed{
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_combine() : assoc array with diff values to both \$keys and \$values argument ***\n";


// get a resource variable
$fp = fopen(__FILE__, "r");

// get a heredoc string
$heredoc = <<<EOT
Hello world
EOT;

// different variations of associative array
$arrays = varray [

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

       // array with object and resource variable
/*10*/ dict[11 => new classA(), "resource" => $fp],

       // array with mixed values
/*11*/ dict[1 => 'hello', 2 => new classA(), 222 => "fruit",
             'resource' => $fp, "int" => 133, "float" => 444.432,
             "heredoc" => $heredoc]
];


// loop through each sub-array within $arrays to check the behavior of array_combine()
$iterator = 1;
foreach($arrays as $array) {
  echo "-- Iteration $iterator --\n";
  var_dump( array_combine($array, $array) );
  $iterator++;
}

// close the file resource used
fclose($fp);

echo "Done";
}
