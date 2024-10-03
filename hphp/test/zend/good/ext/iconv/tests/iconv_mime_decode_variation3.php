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
// Some of the parameters actually passed to charset will request to use
// a default charset determined by the platform. In order for this test to
// run on both linux and windows, the subject will have to be ascii only.
$header = b'Subject: =?UTF-8?B?QSBTYW1wbGUgVGVzdA==?=';
$mode = ICONV_MIME_DECODE_CONTINUE_ON_ERROR;
$charset = 'UTF-8';



// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to $str argument
$inputs = vec[
       // null data
/*1*/  NULL,
       null,

       // empty data
/*3*/  "",
       '',

       // string data
/*5*/  "string",
       'string',
       $heredoc,
];

// loop through each element of $inputs to check the behavior of iconv_mime_decode()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  $res = false;
  try { $res = iconv_mime_decode($header, $mode, $input); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  if ($res !== false) {
       var_dump(bin2hex($res));
  }
  else {
     var_dump($res);
  }
  $iterator++;
};

echo "Done";
}
