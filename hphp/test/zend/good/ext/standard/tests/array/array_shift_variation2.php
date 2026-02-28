<?hh
/* Prototype  : mixed array_shift(array &$stack)
 * Description: Pops an element off the beginning of the array
 * Source code: ext/standard/array.c
 */

/*
 * Pass arrays where values are of one data type to test behaviour of array_shift()
 */

// get a class
class classA
{
  public function __toString() :mixed{
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_shift() : usage variations ***\n";



// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// arrays of different data types to be passed to $stack argument
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

// loop through each element of $inputs to check the behavior of array_shift
$iterator = 1;
foreach($inputs as $key => $input) {
  echo "\n-- Iteration $iterator: $key data --\n";
  var_dump( array_shift(inout $input) );
  var_dump($input);
  $iterator++;
};

fclose($fp);


echo "Done";
}
