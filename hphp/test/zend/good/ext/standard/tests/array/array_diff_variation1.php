<?hh
/* Prototype  : array array_diff(array $arr1, array $arr2 [, array ...])
 * Description: Returns the entries of $arr1 that have values which are not
 * present in any of the others arguments.
 * Source code: ext/standard/array.c
 */

/*
 * Test array_diff by passing non array values in place of $arr1
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

$array = vec[1, 2, 3];


// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to $input argument
$inputs = vec[

       // int data
/*1*/  0,
       1,
       12345,
       -2345,

       // float data
/*5*/  10.5,
       -10.5,
       12.3456789000e10,
       12.3456789000E-10,
       .5,

       // null data
/*10*/ NULL,
       null,

       // boolean data
/*12*/ true,
       false,
       TRUE,
       FALSE,

       // empty data
/*16*/ "",
       '',

       // string data
/*18*/ "string",
       'string',
       $heredoc,

       // binary data
/*21*/ b"binary",
       (string)"binary",

       // object data
/*23*/ new classA(),



       // resource variable
/*24*/ $fp
];

// loop through each element of $inputs to check the behavior of array_diff
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --";
  var_dump( array_diff($input, $array));
  $iterator++;
};

fclose($fp);
echo "Done";
}
