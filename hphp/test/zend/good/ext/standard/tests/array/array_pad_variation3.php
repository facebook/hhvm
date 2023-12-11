<?hh
/* Prototype  : array array_pad(array $input, int $pad_size, mixed $pad_value)
 * Description: Returns a copy of input array padded with pad_value to size pad_size
 * Source code: ext/standard/array.c
*/

/*
* Testing array_pad() function for expected behavior by passing
* different possible values for $pad_value argument.
* $input and $pad_size arguments take fixed value.
*/

// get a class
class classA
{
  public function __toString() :mixed{
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_pad() : possible values for \$pad_value argument ***\n";

// Initialise $input and $pad_size argument
$input = vec[1, 2];
$pad_size = 4;

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// different values to be passed to $pad_value argument
$pad_values = vec[

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

       // array data
/*10*/ vec[],
       vec[0],
       vec[1],
       vec[1, 2],
       dict['color' => 'red', 'item' => 'pen'],

       // null data
/*15*/ NULL,
       null,

       // boolean data
/*17*/ true,
       false,
       TRUE,
       FALSE,

       // empty data
/*21*/ "",
       '',

       // string data
/*23*/ "string",
       'string',
       $heredoc,

       // strings with different white spaces
/*26*/ "\v\fHello\t world!!\rstring\n",
       '\v\fHello\t world!!\rstring\n',

       // object data
/*28*/ new classA(),

       // resource variable
/*29*/ $fp,
];

// loop through each element of $pad_values to check the behavior of array_pad()
$iterator = 1;
foreach($pad_values as $pad_value) {
  echo "-- Iteration $iterator --\n";
  var_dump( array_pad($input, $pad_size, $pad_value) );  // positive 'pad_size'
  var_dump( array_pad($input, -$pad_size, $pad_value) );  // negative 'pad_size'
  $iterator++;
};

echo "Done";
}
