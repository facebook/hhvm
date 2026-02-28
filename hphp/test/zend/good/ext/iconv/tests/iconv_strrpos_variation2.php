<?hh
/* Prototype  : proto int iconv_strrpos(string haystack, string needle [, string charset])
 * Description: Find position of last occurrence of a string within another
 * Source code: ext/iconv/iconv.c
 */

/*
 * Pass iconv_strrpos() different data types as $needle argument to test behaviour
 */

// get a class
class classA
{
  public function __toString() :mixed{
    return "world";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing iconv_strrpos() : usage variations ***\n";

// Initialise function arguments not being substituted
$haystack = 'hello, world';
$encoding = 'utf-8';


// heredoc string
$heredoc = <<<EOT
world
EOT;

// unexpected values to be passed to $needle argument
$inputs = vec[
       // empty data
/*1*/  "",
       '',

       // string data
/*3*/  "world",
       'world',
       $heredoc,
];

// loop through each element of $inputs to check the behavior of iconv_strrpos()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  try { var_dump( iconv_strrpos($haystack, $input, $encoding)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

echo "Done";
}
