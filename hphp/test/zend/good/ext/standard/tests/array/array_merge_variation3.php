<?hh
/* Prototype  : array array_merge(array $arr1, array $arr2 [, array $...])
 * Description: Merges elements from passed arrays into one array
 * Source code: ext/standard/array.c
 */

/*
 * Pass arrays of different data types to test how array_merge adds them
 * onto an existing array
 */

// get a class
class classA
{
  public function __toString() :mixed{
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_merge() : usage variations ***\n";

// Initialise function arguments not being substituted
$arr = varray [1, 2];


// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// arrays of different data types to be passed as $input
$inputs = dict[

       // int data
/*1*/  'int' => vec[
       0,
       1,
       12345,
       -2345,
       ],

       // float data
/*2*/  'float' => vec[
       10.5,
       -10.5,
       12.3456789000e10,
       12.3456789000E-10,
       .5,
       ],

       // null data
/*3*/ 'null' => vec[
       NULL,
       null,
       ],

       // boolean data
/*4*/ 'bool' => vec[
       true,
       false,
       TRUE,
       FALSE,
       ],

       // empty data
/*5*/ 'empty string' => vec[
       "",
       '',
       ],

/*6*/ 'empty array' => vec[
       ],

       // string data
/*7*/ 'string' => vec[
       "string",
       'string',
       $heredoc,
       ],

       // object data
/*8*/ 'object' => vec[
       new classA(),
       ],

       // resource variable
/*9*/ 'resource' => vec[
       $fp
       ],
];

// loop through each element of $inputs to check the behavior of array_merge
$iterator = 1;
foreach($inputs as $key => $input) {
  echo "\n-- Iteration $iterator: $key data --\n";
  var_dump( array_merge($input, $arr) );
  var_dump( array_merge($arr, $input) );
  $iterator++;
};

fclose($fp);

echo "Done";
}
