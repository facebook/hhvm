<?php
/* Prototype  : bool iconv_set_encoding(string type, string charset)
 * Description: Sets internal encoding and output encoding for ob_iconv_handler()
 * Source code: ext/iconv/iconv.c 
 */

/*
 * Test Error functionality of iconv_get_encoding
 */

echo "*** Testing iconv_set_encoding() : error functionality ***\n";

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// get a class
class classA
{
  public function __toString() {
    return "UTF-8";
  }
}

// heredoc string
$heredoc = <<<EOT
Nothing
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to $encoding argument
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

       // invalid string data
/*18*/ "Nothing",
       'Nothing',
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

// loop through each element of $inputs to check the behavior of mb_regex_encoding()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  var_dump( iconv_set_encoding($input, "UTF-8") );
  $iterator++;
};

fclose($fp);

echo "Done";
?>