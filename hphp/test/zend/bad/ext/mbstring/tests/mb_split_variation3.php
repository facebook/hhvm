<?php
/* Prototype  : proto array mb_split(string pattern, string string [, int limit])
 * Description: split multibyte string into array by regular expression 
 * Source code: ext/mbstring/php_mbregex.c
 * Alias to functions: 
 */

echo "*** Testing mb_split() : usage variations ***\n";

// Initialise function arguments not being substituted (if any)
$pattern = '[a-z]';
$string = 'string_val';

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
UTF-8
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

       // string data
/*18*/ "UTF-8",
       'UTF-8',
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

// loop through each element of the array for pattern

$iterator = 1;
foreach($inputs as $input) {
      echo "\n-- Iteration $iterator --\n";
      var_dump( mb_split($pattern, $string, $input) );
      $iterator++;
};

fclose($fp);
echo "Done";
?>