<?hh
/* Prototype  : array array_change_key_case(array $input [, int $case])
 * Description: Retuns an array with all string keys lowercased [or uppercased]
 * Source code: ext/standard/array.c
 */

/*
 * Pass arrays with different data types as keys to array_change_key_case()
 * to test conversion
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_change_key_case() : usage variations ***\n";


// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// arrays of different data types to be passed to $input argument
$inputs = darray[

       // int data
/*1*/  'int' => darray[
       0 => 'zero',
       1 => 'one',
       12345 => 'positive',
       -2345 => 'negative',
       ],

       // empty data
/*5*/ 'empty double quotes' => darray[
       "" => 'emptyd',
       ],
       'empty single quotes' => darray[
       '' => 'emptys',
       ],

       // string data
/*6*/ 'string' => darray[
       "stringd" => 'stringd',
       'strings' => 'strings',
       $heredoc => 'stringh',
       ],
];

// loop through each sub-array of $inputs to check the behavior of array_change_key_case()
$iterator = 1;
foreach($inputs as $key => $input) {
  echo "\n-- Iteration $iterator : $key data --\n";
  var_dump( array_change_key_case($input, CASE_UPPER) );
  $iterator++;
};

echo "Done";
}
