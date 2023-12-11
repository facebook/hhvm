<?hh
/* Prototype  : array array_merge(array $arr1, array $arr2 [, array $...])
 * Description: Merges elements from passed arrays into one array
 * Source code: ext/standard/array.c
 */

/*
 * Pass an array with different data types as keys to test how array_merge
 * adds it onto an existing array
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_merge() : usage variations ***\n";

// Initialise function arguments not being substituted
$arr = darray ['one' => 1, 'two' => 2];

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// arrays with keys as different data types to be passed as $input
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

// loop through each element of $inputs to check the behavior of array_merge
$iterator = 1;
foreach($inputs as $key => $input) {
  echo "\n-- Iteration $iterator: $key data --\n";
  var_dump( array_merge($input, $arr) );
  var_dump( array_merge($arr, $input) );
  $iterator++;
};

echo "Done";
}
