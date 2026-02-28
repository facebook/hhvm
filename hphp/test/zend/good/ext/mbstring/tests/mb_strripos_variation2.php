<?hh
/* Prototype  : int mb_strripos(string haystack, string needle [, int offset [, string encoding]])
 * Description: Finds position of last occurrence of a string within another, case insensitive
 * Source code: ext/mbstring/mbstring.c
 * Alias to functions:
 */

/*
 * Pass mb_strripos different data types as $needle arg to test behaviour
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
$haystack = b'string_val';
$offset = 0;
$encoding = 'utf-8';


// heredoc string
$heredoc = b<<<EOT
hello world
EOT;

// unexpected values to be passed to $needle argument
$inputs = vec[
       // empty data
/*16*/ "",
       '',

       // string data
/*18*/ b"string",
       b'string',
       $heredoc,
];

// loop through each element of $inputs to check the behavior of mb_strripos()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  try { var_dump( mb_strripos($haystack, $input, $offset, $encoding)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

echo "Done";
}
