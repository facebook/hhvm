<?hh
/* Prototype  : bool array_key_exists(mixed $key, array $search)
 * Description: Checks if the given key or index exists in the array
 * Source code: ext/standard/array.c
 * Alias to functions: key_exists
 */

/*
 * Pass different data types as $key argument to array_key_exists() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_key_exists() : usage variations ***\n";

// Initialise function arguments not being substituted
$search = dict[0 => 'zero', 'key' => 'val', 1 => 'two'];

// heredoc string
$heredoc = <<<EOT
key
EOT;

// unexpected values to be passed to $key argument
$inputs = vec[

       // int data
/*1*/  0,
       1,
       12345,
       -2345,

       // empty data
/*5*/  "",
       '',

       // string data
/*7*/  "key",
       'key',
       $heredoc,

];

// loop through each element of $inputs to check the behavior of array_key_exists()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  var_dump( array_key_exists($input, $search) );
  $iterator++;
};

echo "Done";
}
