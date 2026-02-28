<?hh
/* Prototype  : array array_change_key_case(array $input [, int $case])
 * Description: Retuns an array with all string keys lowercased [or uppercased]
 * Source code: ext/standard/array.c
 */

/*
 * Pass different data types as $case argument to array_change_key_case() to test behaviour
 * Where possible, CASE_UPPER has been entered as a string value
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_change_key_case() : usage variations ***\n";

// Initialise function arguments not being substituted
$array = dict['one' => 1, 'TWO' => 2, 'Three' => 3];


// heredoc string
$heredoc = <<<EOT
CASE_UPPER
EOT;

// unexpected values to be passed to $case argument
$inputs = vec[
       // int data
/*1*/  0,
       1,
       12345,
       -2345,
];

// loop through each element of $inputs to check the behavior of array_change_key_case()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  try { var_dump( array_change_key_case($array, $input) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

echo "Done";
}
