<?hh
/* Prototype  : mixed key(array $array_arg)
 * Description: Return the key of the element currently pointed to by the internal array pointer
 * Source code: ext/standard/array.c
 */

/*
 * Pass arrays where keys are different data types as $array_arg to key() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing key() : usage variations ***\n";

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// unexpected values to be passed as $array_arg
$inputs = darray[

       // int data
/*1*/  'int' => darray[
       0 => 'zero',
       1 => 'one',
       12345 => 'positive',
       -2345 => 'negative',
       ],

       // empty data
/*2*/ 'empty double quotes' => darray[
       "" => 'emptyd',
       ],

/*3*/  'empty single quotes' => darray[
       '' => 'emptys',
       ],

       // string data
/*4*/  'string' => darray[
       "stringd" => 'stringd',
       'strings' => 'strings',
       $heredoc => 'stringh',
       ],
];

// loop through each element of $inputs to check the behavior of key()
$iterator = 1;
foreach($inputs as $key => $input) {
  echo "\n-- Iteration $iterator : $key data --\n";
  while (key($input) !== NULL) {
      var_dump(key($input));
      next(inout $input);
  }
  $iterator++;
};
echo "===DONE===\n";
}
