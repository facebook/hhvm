<?hh
/* Prototype  : array array_values(array $input)
 * Description: Return just the values from the input array
 * Source code: ext/standard/array.c
 */

/*
 * Pass arrays where the keys are different data types as $input argument
 * to array_values() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_values() : usage variations ***\n";

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// unexpected values to be passed as $input
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

/*3*/  'empty single quotes' => dict[
       '' => 'emptys',
       ],

       // string data
/*4*/  'string' => dict[
       "stringd" => 'stringd',
       'strings' => 'strings',
       $heredoc => 'stringh',
       ],
];

// loop through each element of $inputs to check the behavior of array_values()
$iterator = 1;
foreach($inputs as $key => $input) {
  echo "\n-- Iteration $iterator: $key data --\n";
  var_dump( array_values($input) );
  $iterator++;
};
echo "Done";
}
