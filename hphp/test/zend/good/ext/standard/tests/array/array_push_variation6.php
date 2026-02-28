<?hh
/* Prototype  : int array_push(&array $stack, mixed $var [, mixed $...])
 * Description: Pushes elements onto the end of the array
 * Source code: ext/standard/array.c
 */

/*
 * Pass array_push arrays where the keys are different data types.
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_push() : usage variations ***\n";

// Initialise function arguments not being substituted
$var = 'value';

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// arrays of different data types as keys to be passed to $stack argument
$inputs = dict[

       // int data
/*1*/  'int' => dict[
       0 => 'zero',
       1 => 'one',
       12345 => 'positive',
       -2345 => 'negative',
       ],

       // empty data
/*2*/ 'empty double quotes' => dict[
       "" => 'emptyd',
       ],
       'empty single quotes' => dict[
       '' => 'emptys',
       ],

       // string data
/*3*/ 'string' => dict[
       "stringd" => 'stringd',
       'strings' => 'strings',
       $heredoc => 'stringh',
       ],
];

// loop through each sub-array of $inputs to check the behavior of array_push()
$iterator = 1;
foreach($inputs as $key => $input) {
  echo "\n-- Iteration $iterator : $key data --\n";
  echo "Before : ";
  var_dump(count($input));
  echo "After  : ";
  var_dump( array_push(inout $input, $var) );
  $iterator++;
};

echo "Done";
}
