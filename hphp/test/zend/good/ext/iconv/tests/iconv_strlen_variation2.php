<?hh
/* Prototype  : int iconv_strlen(string str [, string charset])
 * Description: Get character numbers of a string
 * Source code: ext/iconv/iconv.c
 */

/*
 * Test iconv_strlen() by passing different data types as $encoding argument.
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
echo "*** Testing iconv_strlen() : usage variations ***\n";

// Initialise function arguments not being substituted
$str = 'string value';


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
/*3*/  "",
       '',

       // string data
/*5*/  "UTF-8",
       'UTF-8',
       $heredoc,
];

// loop through each element of $inputs to check the behavior of iconv_strlen()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  try { var_dump( iconv_strlen($str, $input)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

echo "Done";
}
