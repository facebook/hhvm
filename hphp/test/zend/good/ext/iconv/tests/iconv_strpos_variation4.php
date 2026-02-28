<?hh
/* Prototype  : int iconv_strpos(string haystack, string needle [, int offset [, string charset]])
 * Description: Find position of first occurrence of a string within another
 * Source code: ext/iconv/iconv.c
 */


/*
 * Pass iconv_strpos different data types as $encoding arg to test behaviour
 * Where possible 'UTF-8' has been entered as a string value
 */

// get a class
class classA
{
  public function __toString() :mixed{
    return "UTF-8";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing iconv_strpos() : usage variations ***\n";

// Initialise function arguments not being substituted
$haystack = b'string_val';
$needle = b'val';
$offset = 0;


// heredoc string
$heredoc = <<<EOT
UTF-8
EOT;

// unexpected values to be passed to $input argument
$inputs = vec[
       // null data
/*1*/  NULL,
       null,

       // empty data
/*3*/ "",
       '',

       // string data
/*5*/ "UTF-8",
       'UTF-8',
       $heredoc,
];

// loop through each element of $inputs to check the behavior of iconv_strpos()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  try { var_dump( iconv_strpos($haystack, $needle, $offset, $input)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

echo "Done";
}
