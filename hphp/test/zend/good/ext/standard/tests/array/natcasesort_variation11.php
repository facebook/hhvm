<?hh
/* Prototype  : bool natcasesort(array &$array_arg)
 * Description: Sort an array using case-insensitive natural sort
 * Source code: ext/standard/array.c
 */

/*
 * Pass arrays where the keys are different data types to test behaviour of natcasesort()
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing natcasesort() : usage variations ***\n";

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// arrays with keys as different data types to be passed as $array_arg
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

       // duplicate values
/*5*/  'duplicate' => dict[
       'foo' => 'bar',
       'baz' => 'bar',
       'hello' => 'world'
       ],
];

// loop through each element of $inputs to check the behavior of natcasesort()
$iterator = 1;
foreach($inputs as $input) {
    echo "\n-- Iteration $iterator --\n";
    var_dump( natcasesort(inout $input) );
    var_dump($input);
    $iterator++;
};

echo "Done";
}
