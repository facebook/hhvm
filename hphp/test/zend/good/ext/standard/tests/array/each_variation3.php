<?hh
/* Prototype  : array each(&array $arr)
 * Description: Return the currently pointed key..value pair in the passed array,
 * and advance the pointer to the next element
 * Source code: Zend/zend_builtin_functions.c
 */

/*
 * Pass each() arrays where the keys are different data types to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing each() : usage variations ***\n";

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// unexpected values to be passed as $arr
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

// loop through each element of $inputs to check the behavior of each()
$iterator = 1;
foreach($inputs as $key => $input) {
  echo "\n-- Iteration $iterator: $key data --\n";
  var_dump( each(inout $input) );
  $iterator++;
};

echo "Done";
}
