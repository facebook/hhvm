<?hh
/* Prototype  : array array_merge_recursive(array $arr1[, array $...])
 * Description: Recursively merges elements from passed arrays into one array
 * Source code: ext/standard/array.c
*/

/*
 * Passing non array values to 'arr2' argument of array_merge_recursive() and see
 * that the function outputs proper warning messages wherever expected.
*/

class A
{
//  public $var = 10;
  public function __toString() :mixed{
    return "object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_merge_recursive() : Passing non array values to \$arr2 argument ***\n";

// initialise the first argument
$arr1 = vec[1, vec["hello", 'world']];


// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to $arr2 argument
$arrays = varray [

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



       // resource variable
/*21*/ $fp,

       // object data
/*22*/ new A()
];

// loop through each element of $arrays and check the behavior of array_merge_recursive()
$iterator = 1;
foreach($arrays as $arr2) {
  echo "\n-- Iteration $iterator --";
  var_dump( array_merge_recursive($arr1, $arr2) );
  $iterator++;
}

// close the file resource used
fclose($fp);

echo "Done";
}
