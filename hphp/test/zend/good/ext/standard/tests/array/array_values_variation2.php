<?hh
/* Prototype  : array array_values(array $input)
 * Description: Return just the values from the input array
 * Source code: ext/standard/array.c
 */

/*
 * Pass arrays of different data types as $input argument to array_values() to test behaviour
 */

// get a class
class classA
{
  public function __toString() {
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_values() : usage variations ***\n";

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// arrays of different data types to be passed as $input
$inputs = darray[

       // int data
/*1*/  'int' => varray[
       0,
       1,
       12345,
       -2345,
       ],

       // float data
/*2*/  'float' => varray[
       10.5,
       -10.5,
       12.3456789000e10,
       12.3456789000E-10,
       .5,
       ],

       // null data
/*3*/ 'null' => varray[
       NULL,
       null,
       ],

       // boolean data
/*4*/ 'bool' => varray[
       true,
       false,
       TRUE,
       FALSE,
       ],

       // empty data
/*5*/ 'empty string' => varray[
       "",
       '',
       ],

/*6*/ 'empty array' => varray[
       ],

       // string data
/*7*/ 'string' => varray[
       "string",
       'string',
       $heredoc,
       ],

       // object data
/*8*/ 'object' => varray[
       new classA(),
       ],

       // undefined data
/*9*/ 'undefined' => varray[
       @$undefined_var,
       ],

       // unset data
/*10*/ 'unset' => varray[
       @$unset_var,
       ],

       // resource variable
/*11*/ 'resource' => varray[
       $fp
       ],
];

// loop through each element of $inputs to check the behavior of array_values()
$iterator = 1;
foreach($inputs as $key => $input) {
  echo "\n-- Iteration $iterator: $key data --\n";
  var_dump( array_values($input) );
  $iterator++;
};

fclose($fp);

echo "Done";
}
