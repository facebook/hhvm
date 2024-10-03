<?hh
/* Prototype  : int iconv_strlen(string str [, string charset])
 * Description: Get character numbers of a string
 * Source code: ext/iconv/iconv.c
 */

/*
 * Test iconv_strlen by passing different data types as $str argument
 */

// get a class
class classA
{
  public function __toString() :mixed{
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing iconv_strlen() : usage variations ***\n";

// Initialise function arguments not being substituted
$encoding = 'utf-8';


// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// unexpected values to be passed to $str argument
$inputs = dict[
       // empty data
/*16*/
      'empty string DQ' => "",
      'empty string SQ' => '',

       // string data
/*18*/
      'string DQ' => "string",
      'string SQ' => 'string',
      'mixed case string' => "sTrInG",
      'heredoc' => $heredoc,
];

// loop through each element of $inputs to check the behavior of iconv_strlen()
$iterator = 1;
foreach($inputs as $key =>$value) {
  echo "\n--$key--\n";
  try { var_dump( iconv_strlen($value, $encoding)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

echo "===DONE===\n";
}
