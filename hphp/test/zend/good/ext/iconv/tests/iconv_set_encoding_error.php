<?hh
/* Prototype  : bool iconv_set_encoding(string type, string charset)
 * Description: Sets internal encoding and output encoding for ob_iconv_handler()
 * Source code: ext/iconv/iconv.c
 */

/*
 * Test Error functionality of iconv_get_encoding
 */

// get a class
class classA
{
  public function __toString() :mixed{
    return "UTF-8";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing iconv_set_encoding() : error functionality ***\n";


// heredoc string
$heredoc = <<<EOT
Nothing
EOT;

// unexpected values to be passed to $encoding argument
$inputs = vec[
       // empty data
/*16*/ "",
       '',

       // invalid string data
/*18*/ "Nothing",
       'Nothing',
       $heredoc,
];

// loop through each element of $inputs to check the behavior of mb_regex_encoding()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  try { var_dump( iconv_set_encoding($input, "UTF-8") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

echo "Done";
}
