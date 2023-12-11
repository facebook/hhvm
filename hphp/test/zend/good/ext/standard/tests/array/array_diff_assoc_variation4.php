<?hh
/* Prototype  : array array_diff_assoc(array $arr1, array $arr2 [, array ...])
 * Description: Returns the entries of arr1 that have values which are not present
 * in any of the others arguments but do additional checks whether the keys are equal
 * Source code: ext/standard/array.c
 */

/*
 * Test how array_diff_assoc() compares arrays containing different data types
 * as keys
 */
<<__EntryPoint>> function main(): void {
echo "\n*** Testing array_diff_assoc() : usage variations ***\n";

$array = vec[1, 2, 3];

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

//Different data types as keys to be passed to $arr1 argument
$inputs = dict[

       // int data
/*1*/
'int' => dict[
       0 => 'zero',
       1 => 'one',
       12345 => 'positive',
       -2345 => 'negative'],

       // empty data
/*2*/
'empty' => dict[
      "" => 'emptyd',
      '' => 'emptys'],

       // string data
/*3*/
'string' => dict[
      "string" => 'stringd',
      'string' => 'strings',
      $heredoc => 'stringh'],

       // binary data
/*4*/
'binary' => dict[
      b"binary1" => 'binary 1',
      (string)"binary2" => 'binary 2'],
];

// loop through each element of $inputs to check the behavior of array_diff_assoc
$iterator = 1;
foreach($inputs as $key => $input) {
  echo "\n-- Iteration $iterator --\n";
  var_dump( array_diff_assoc($input, $array));
  $iterator++;
};

echo "Done";
}
