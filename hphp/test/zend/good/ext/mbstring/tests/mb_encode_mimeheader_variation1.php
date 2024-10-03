<?hh
/* Prototype  : string mb_encode_mimeheader
 * (string $str [, string $charset [, string $transfer_encoding [, string $linefeed [, int $indent]]]])
 * Description: Converts the string to MIME "encoded-word" in the format of =?charset?(B|Q)?encoded_string?=
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Pass different data types to $str argument to see how mb_encode_mimeheader() behaves
 */

// get a class
class classA
{
  public function __toString() :mixed{
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_encode_mimeheader() : usage variations ***\n";

// Initialise function arguments not being substituted
$charset = 'utf-8';
$transfer_encoding = 'B';
$linefeed = "\r\n";
$indent = 2;


// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// unexpected values to be passed to $str argument
$inputs = vec[
       // empty data
/*16*/ "",
       '',

       // string data
/*18*/ "string",
       'string',
       $heredoc,
];

// loop through each element of $inputs to check the behavior of mb_encode_mimeheader()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  try { var_dump( mb_encode_mimeheader($input, $charset, $transfer_encoding, $linefeed, $indent)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

echo "Done";
}
