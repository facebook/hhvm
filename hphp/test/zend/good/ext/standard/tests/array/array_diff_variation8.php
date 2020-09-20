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
  public function __toString() {
     return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff() : usage variations ***\n";

$array = darray['a' => '1', 'b' => '2', 'c' => '3'];

// get an unset variable
$unset_var = 10;
unset ($unset_var);

// get a resource variable
$fp = fopen(__FILE__, "r");

// get a heredoc string
$heredoc = <<<EOT
Hello world
EOT;

// associative arrays with different values
$inputs = varray [
       // arrays with integer values
/*1*/  darray['0' => 0, '1' => 0],
       darray["one" => 1, 'two' => 2, "three" => 1, 4 => 1],

       // arrays with float values
/*3*/  darray["float1" => 2.3333, "float2" => 2.3333],
       darray["f1" => 1.2, 'f2' => 3.33, 3 => 4.89999922839999, 'f4' => 1.2],

       // arrays with string values
/*5*/  darray[111 => "\tHello", "red" => "col\tor", 2 => "\v\fworld", 3 =>  "\tHello"],
       darray[111 => '\tHello', "red" => 'col\tor', 2 => '\v\fworld', 3 =>  '\tHello'],
       darray[1 => "hello", "heredoc" => $heredoc, 2 => $heredoc],

       // array with object, unset variable and resource variable
/*8*/ darray[11 => new classA(), "unset" => @$unset_var, "resource" => $fp, 12 => new classA(), 13 => $fp],
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
