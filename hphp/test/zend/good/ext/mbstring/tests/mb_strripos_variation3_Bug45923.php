<?hh
/* Prototype  : int mb_strripos(string haystack, string needle [, int offset [, string encoding]])
 * Description: Finds position of last occurrence of a string within another, case insensitive
 * Source code: ext/mbstring/mbstring.c
 * Alias to functions:
 */

/*
 * Pass mb_strripos different data types as $offset arg to test behaviour
 */

// get a class
class classA
{
  public function __toString() :mixed{
    return b"Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_strripos() : usage variations ***\n";

// Initialise function arguments not being substituted
$needle = b'A';
$haystack = b'string_val';
$encoding = 'utf-8';

// unexpected values to be passed to $offest argument
$inputs = vec[
       // int data
/*1*/  0,
       1,
       12345,
       -2345,
];

// loop through each element of $inputs to check the behavior of mb_strripos()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  try { var_dump( mb_strripos($haystack, $needle, $input, $encoding)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

echo "Done";
}
