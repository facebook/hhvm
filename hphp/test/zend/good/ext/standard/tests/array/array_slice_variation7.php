<?hh
/* Prototype  : array array_slice(array $input, int $offset [, int $length [, bool $preserve_keys]])
 * Description: Returns elements specified by offset and length 
 * Source code: ext/standard/array.c
 */

/*
 * Pass different data types as keys in an array to array_slice()
 * to test how $preserve_keys treats them
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_slice() : usage variations ***\n";

// Initialise function arguments not being substituted
$offset = 0;
$length = 10; // to ensure all elements are displayed

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// arrays of different data types to be passed as $input
$inputs = darray[

       // int data
/*1*/  'int' => darray[
       0 => 'zero',
       1 => 'one',
       12345 => 'positive',
       -2345 => 'negative',
       ],

       // float data
/*2*/  'float' => darray[
       10.5 => 'positive',
       -10.5 => 'negative',
       .5 => 'half',
       ],
       
/*3*/  'extreme floats' => darray[
       12.3456789000e6 => 'large',
       12.3456789000E-10 => 'small',
       ],

       // null data
/*4*/ 'null uppercase' => darray[
       NULL => 'null 1',
       ], 
       
/*5*/  'null lowercase' => darray[
       null => 'null 2',
       ],

       // boolean data
/*6*/ 'bool lowercase' => darray[
       true => 'lowert',
       false => 'lowerf',
       ],
       
/*7*/  'bool uppercase' => darray[
       TRUE => 'uppert',
       FALSE => 'upperf',
       ],
       
       // empty data
/*8*/ 'empty double quotes' => darray[
       "" => 'emptyd',
       ],
       
/*9*/  'empty single quotes' => darray[
       '' => 'emptys',
       ],

       // string data
/*10*/ 'string' => darray[
       "stringd" => 'stringd',
       'strings' => 'strings',
       $heredoc => 'stringh',
       ],

       // undefined data
/*11*/ 'undefined' => darray[
       @$undefined_var => 'undefined',
       ],

       // unset data
/*12*/ 'unset' => darray[
       @$unset_var => 'unset',
       ],
];

// loop through each element of $inputs to check the behavior of array_slice()
$iterator = 1;
foreach($inputs as $type => $input) {
  echo "\n-- Iteration $iterator : key type is $type --\n";
  echo "\$preserve_keys = TRUE\n";
  var_dump( array_slice($input, $offset, $length, true) );
  echo "\$preserve_keys = FALSE\n";
  var_dump( array_slice($input, $offset, $length, false) );
  $iterator++;
};

echo "Done";
}
