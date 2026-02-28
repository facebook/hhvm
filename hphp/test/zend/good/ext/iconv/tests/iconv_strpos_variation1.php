<?hh
/* Prototype  : int iconv_strpos(string haystack, string needle [, int offset [, string charset]])
 * Description: Find position of first occurrence of a string within another
 * Source code: ext/iconv/iconv.c
 */

/*
 * Pass iconv_strpos different data types as $haystack arg to test behaviour
 */

// get a class
class classA
{
  public function __toString() :mixed{
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing iconv_strpos() : usage variations ***\n";

// Initialise function arguments not being substituted
$needle = 'string_val';
$offset = 0;
$encoding = 'utf-8';


// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// unexpected values to be passed to $haystack argument
$inputs = vec[
       // empty data
/*1*/  "",
       '',

       // string data
/*3*/  "string",
       'string',
       $heredoc,
];

// loop through each element of $inputs to check the behavior of iconv_strpos()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  try { var_dump( iconv_strpos($input, $needle, $offset, $encoding)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

echo "Done";
}
