<?hh
/* Prototype  : mixed array_shift(array &$stack)
 * Description: Pops an element off the beginning of the array
 * Source code: ext/standard/array.c
 */

/*
 * Pass arrays with different data types as keys to test how array_shift() re-assigns keys
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_shift() : usage variations ***\n";

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// unexpected values to be passed to $stack argument
$inputs = dict[

       // int data
/*1*/  'int' => dict[
       0 => 'zero',
       1 => 'one',
       12345 => 'positive',
       -2345 => 'negative',
       ],

       // empty data
/*2*/  'empty double quotes' => dict[
       "" => 'emptyd',
       ],

/*3*/  'empty single quotes' => dict[
       '' => 'emptys',
       ],

       // string data
/*4*/ 'string' => dict[
       "stringd" => 'stringd',
       'strings' => 'strings',
       $heredoc => 'stringh',
       ],
];

// loop through each element of $inputs to check the behavior of array_shift()
$iterator = 1;
foreach($inputs as $key => $input) {
  echo "\n-- Iteration $iterator : $key data --\n";
  var_dump( array_shift(inout $input) );
  var_dump($input);
  $iterator++;
};

echo "Done";
}
