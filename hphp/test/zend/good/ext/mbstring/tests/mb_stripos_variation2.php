<?hh
/* Prototype  : int mb_stripos(string haystack, string needle [, int offset [, string encoding]])
 * Description: Finds position of first occurrence of a string within another, case insensitive
 * Source code: ext/mbstring/mbstring.c
 * Alias to functions:
 */

/*
 * Pass mb_stripos different data types as $needle arg to test behaviour
 */

// get a class
class classA
{
  public function __toString() :mixed{
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_stripos() : usage variations ***\n";

// Initialise function arguments not being substituted
$haystack = b'string_val';
$offset = 0;
$encoding = 'utf-8';


// heredoc string
$heredoc = <<<EOT
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

// loop through each element of $inputs to check the behavior of mb_stripos()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  try { var_dump( mb_stripos($haystack, $input, $offset, $encoding)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

echo "Done";
}
