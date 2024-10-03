<?hh
/* Prototype  : proto int iconv_strrpos(string haystack, string needle [, string charset])
 * Description: Find position of last occurrence of a string within another
 * Source code: ext/iconv/iconv.c
 */

/*
 * Pass iconv_strrpos() different data types as $encoding argument to test behaviour
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
echo "*** Testing iconv_strrpos() : usage variations ***\n";

// Initialise function arguments not being substituted
$haystack = b'hello, world';
$needle = b'world';


// heredoc string
$heredoc = <<<EOT
UTF-8
EOT;

// unexpected values to be passed to $encoding argument
$inputs = vec[
       // null data
/*1*/  NULL,
       null,

       // empty data
/*3*/  "",
       '',

       // string data
/*5*/  "UTF-8",
       'UTF-8',
       $heredoc,
];

// loop through each element of $inputs to check the behavior of iconv_strrpos()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  try { var_dump( iconv_strrpos($haystack, $needle, $input)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

echo "Done";
}
