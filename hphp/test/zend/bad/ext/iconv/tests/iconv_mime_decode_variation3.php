<?php
/* Prototype  : string iconv_mime_decode(string encoded_string [, int mode, string charset])
 * Description: Decodes a mime header field
 * Source code: ext/iconv/iconv.c
 */

/*
 * Pass different data types to $str argument to see how iconv_mime_decode() behaves
 */

echo "*** Testing iconv_mime_decode() : usage variations ***\n";

// Initialise function arguments not being substituted
// Some of the parameters actually passed to charset will request to use
// a default charset determined by the platform. In order for this test to
// run on both linux and windows, the subject will have to be ascii only.
$header = b'Subject: =?UTF-8?B?QSBTYW1wbGUgVGVzdA==?=';
$mode = ICONV_MIME_DECODE_CONTINUE_ON_ERROR;
$charset = 'UTF-8';


//get an unset variable
$unset_var = 10;
unset ($unset_var);

// get a class
class classA
{
  public function __toString() {
    return "Class A object";
  }
}

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to $str argument
$inputs = array(

       // int data
/*1*/  0,
       1,
       12345,
       -2345,

       // float data
/*5*/  10.5,
       -10.5,
       12.3456789000e10,
       12.3456789000E-10,
       .5,

       // null data
/*10*/ NULL,
       null,

       // boolean data
/*12*/ true,
       false,
       TRUE,
       FALSE,
       
       // empty data
/*16*/ "",
       '',

       // string data
/*18*/ "string",
       'string',
       $heredoc,
       
       // object data
/*21*/ new classA(),

       // undefined data
/*22*/ @$undefined_var,

       // unset data
/*23*/ @$unset_var,

       // resource variable
/*24*/ $fp
);

// loop through each element of $inputs to check the behavior of iconv_mime_decode()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  $res = iconv_mime_decode($header, $mode, $input);
  if ($res !== false) {
  	 var_dump(bin2hex($res));
  }
  else {
     var_dump($res);
  }
  $iterator++;
};

fclose($fp);

echo "Done";
?>