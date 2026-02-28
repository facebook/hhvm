<?hh
/* Prototype  : array iconv_mime_decode_headers(string headers [, int mode, string charset])
 * Description: Decodes multiple mime header fields
 * Source code: ext/iconv/iconv.c
 */

/*
 * Pass different data types to $str argument to see how iconv_mime_decode_headers() behaves
 */

// get a class
class classA
{
  public function __toString() :mixed{
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing iconv_mime_decode_headers() : usage variations ***\n";

// Initialise function arguments not being substituted
$headers = <<<EOF
Subject: =?UTF-8?B?QSBTYW1wbGUgVGVzdA==?=
To: example@example.com
Date: Thu, 1 Jan 1970 00:00:00 +0000
Message-Id: <example@example.com>
Received: from localhost (localhost [127.0.0.1]) by localhost
    with SMTP id example for <example@example.com>;
    Thu, 1 Jan 1970 00:00:00 +0000 (UTC)
    (envelope-from example-return-0000-example=example.com@example.com)
Received: (qmail 0 invoked by uid 65534); 1 Thu 2003 00:00:00 +0000

EOF;

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

// loop through each element of $inputs to check the behavior of iconv_mime_decode_headers()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  try { var_dump( iconv_mime_decode_headers($headers, $input, $charset)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

echo "Done";
}
