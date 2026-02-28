<?hh
/* Prototype  : bool array_key_exists(mixed $key, array $search)
 * Description: Checks if the given key or index exists in the array
 * Source code: ext/standard/array.c
 * Alias to functions: key_exists
 */

/*
 * Pass an array where the keys are different data types as the $search argument
 * then pass many different data types as $key argument to test where array_key_exist()
 * returns true.
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_key_exists() : usage variations ***\n";

// heredoc string
$heredoc = <<<EOT
string
EOT;

// different data types to be iterated over
$inputs = dict[

       // int data
/*1*/  'int' => dict[
       0 => 'zero',
       1 => 'one',
       12345 => 'positive',
       -2345 => 'negative',
       ],

       // empty data
/*5*/ 'empty double quotes' => dict[
       "" => 'emptyd',
       ],
       'empty single quotes' => dict[
       '' => 'emptys',
       ],

       // string data
/*6*/ 'string' => dict[
       "stringd" => 'stringd',
       'strings' => 'strings',
       $heredoc => 'stringh',
       ],
];

// loop through each element of $inputs to check the behavior of array_key_exists()
$iterator = 1;
foreach($inputs as $type => $input) {
    echo "\n-- Iteration $iterator: $type data --\n";

    //iterate over again to get all different key values
    foreach ($inputs as $new_type => $new_input) {
        echo "-- \$key arguments are $new_type data:\n";
        foreach ($new_input as $key => $search) {
            var_dump(array_key_exists($key, $input));
        }
    }
    $iterator++;
};

echo "Done";
}
