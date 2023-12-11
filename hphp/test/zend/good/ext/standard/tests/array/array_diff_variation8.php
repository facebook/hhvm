<?hh
/* Prototype  : array array_diff(array $arr1, array $arr2 [, array ...])
 * Description: Returns the entries of $arr1 that have values which are
 * not present in any of the others arguments.
 * Source code: ext/standard/array.c
 */

/*
 * Test array_diff() with associative arrays containing different data types as values
 */

// get a class
class classA
{
  public function __toString() :mixed{
     return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff() : usage variations ***\n";

$array = dict['a' => '1', 'b' => '2', 'c' => '3'];


// get a resource variable
$fp = fopen(__FILE__, "r");

// get a heredoc string
$heredoc = <<<EOT
Hello world
EOT;

// associative arrays with different values
$inputs = varray [
       // arrays with integer values
/*1*/  dict['0' => 0, '1' => 0],
       dict["one" => 1, 'two' => 2, "three" => 1, 4 => 1],

       // arrays with float values
/*3*/  dict["float1" => 2.3333, "float2" => 2.3333],
       dict["f1" => 1.2, 'f2' => 3.33, 3 => 4.89999922839999, 'f4' => 1.2],

       // arrays with string values
/*5*/  dict[111 => "\tHello", "red" => "col\tor", 2 => "\v\fworld", 3 =>  "\tHello"],
       dict[111 => '\tHello', "red" => 'col\tor', 2 => '\v\fworld', 3 =>  '\tHello'],
       dict[1 => "hello", "heredoc" => $heredoc, 2 => $heredoc],

       // array with object, unset variable and resource variable
/*8*/ dict[11 => new classA(), "resource" => $fp, 12 => new classA(), 13 => $fp],
];

// loop through each sub-array of $inputs to check the behavior of array_unique()
$iterator = 1;
foreach($inputs as $input) {
  echo "-- Iteration $iterator --\n";
  var_dump( array_diff($array, $input) );
  var_dump( array_diff($input, $array) );
  $iterator++;
}

fclose($fp);

echo "Done";
}
