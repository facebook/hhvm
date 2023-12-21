<?hh
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays
 * Source code: ext/standard/array.c
 */

/*
 * Test array_map() by passing associative array with different values for $arr1 argument
 */

//get a class
class classA
{
  public function __toString():mixed{
    return "Class A object";
  }
}
function callback($a)
:mixed{
  return ($a);
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_map() : associative array with diff. values for 'arr1' argument ***\n";


//get a resource variable
$fp = fopen(__FILE__, "r");

// get a heredoc string
$heredoc = <<<EOT
Hello world
EOT;

// initializing the array
$arrays = vec[

       // empty array
/*1*/  vec[],

       // arrays with integer values
       dict['0' => 0],
       dict["1" => 1],
       dict["one" => 1, 'two' => 2, "three" => 3, 4 => 4],

       // arrays with float values
/*5*/  dict["float" => 2.3333],
       dict["f1" => 1.2, 'f2' => 3.33, 3 => 4.89999922839999, 'f4' => 33333333.3333],

       // arrays with string values
       dict[111 => "\tHello", "red" => "col\tor", 2 => "\v\fworld", 3 =>  "pen\n"],
/*8*/  dict[111 => '\tHello', "red" => 'col\tor', 2 => '\v\fworld', 3 =>  'pen\n'],
       dict[1 => "hello", "heredoc" => $heredoc],

       // array with object, unset variable and resource variable
       dict[11 => new classA(), "resource" => $fp],

       // array with mixed values
/*11*/ dict[1 => 'hello', 2 => new classA(), 222 => "fruit",
             'resource' => $fp, "int" => 133, "float" => 444.432,
             "heredoc" => $heredoc]
];

// loop through the various elements of $arrays to test array_map()
$iterator = 1;
foreach($arrays as $arr1) {
  echo "-- Iteration $iterator --\n";
  var_dump( array_map(callback<>, $arr1) );
  $iterator++;
}

echo "Done";
}
