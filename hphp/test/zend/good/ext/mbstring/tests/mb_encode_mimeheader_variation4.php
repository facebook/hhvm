<?hh
/* Prototype  : string mb_encode_mimeheader
 * (string $str [, string $charset [, string $transfer_encoding [, string $linefeed [, int $indent]]]])
 * Description: Converts the string to MIME "encoded-word" in the format of =?charset?(B|Q)?encoded_string?=
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Pass different data types to $linefeed argument to see how mb_encode_mimeheader() behaves
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
mb_internal_encoding('utf-8');


// Initialise function arguments not being substituted
//longer $str to go over 1 line
$str = base64_decode('zpHPhc+Ez4wgzrXOr869zrHOuSDOtc67zrvOt869zrnOus+MIM66zrXOr868zrXOvc6/LiAwMTIzNDU2Nzg5Lg==');
$charset = 'utf-8';
$transfer_encoding = 'B';
$indent = 2;


// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// unexpected values to be passed to $linefeed argument
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
  try { var_dump( mb_encode_mimeheader($str, $charset, $transfer_encoding, $input, $indent)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

echo "Done";
}
