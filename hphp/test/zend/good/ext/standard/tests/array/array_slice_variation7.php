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

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// arrays of different data types to be passed as $input
$inputs = dict[

       // int data
/*1*/  'int' => dict[
       0 => 'zero',
       1 => 'one',
       12345 => 'positive',
       -2345 => 'negative',
       ],

       // empty data
/*8*/ 'empty double quotes' => dict[
       "" => 'emptyd',
       ],

/*9*/  'empty single quotes' => dict[
       '' => 'emptys',
       ],

       // string data
/*10*/ 'string' => dict[
       "stringd" => 'stringd',
       'strings' => 'strings',
       $heredoc => 'stringh',
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
