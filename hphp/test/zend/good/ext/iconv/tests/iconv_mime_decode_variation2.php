<?hh
/* Prototype  : string iconv_mime_decode(string encoded_string [, int mode, string charset])
 * Description: Decodes a mime header field
 * Source code: ext/iconv/iconv.c
 */

/*
 * Pass different data types to $str argument to see how iconv_mime_decode() behaves
 */

// get a class
class classA
{
  public function __toString() :mixed{
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing iconv_mime_decode() : usage variations ***\n";

// Initialise function arguments not being substituted
$header = b'Subject: =?UTF-8?B?UHLDvGZ1bmcgUHLDvGZ1bmc=?=';
$mode = ICONV_MIME_DECODE_CONTINUE_ON_ERROR;
$charset = 'UTF-8';



// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// unexpected values to be passed to $str argument
$inputs = vec[
       // int data
/*1*/  0,
       1,
       12345,
       -2345,
];

// loop through each element of $inputs to check the behavior of iconv_mime_decode()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  try { var_dump( bin2hex(iconv_mime_decode($header, $input, $charset))); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

echo "Done";
}
