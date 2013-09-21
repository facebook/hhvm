<?php
/* Prototype  : mixed key(array $array_arg)
 * Description: Return the key of the element currently pointed to by the internal array pointer 
 * Source code: ext/standard/array.c
 */

/*
 * Pass different data types as $array_arg argument to test behaviour of key()
 */

echo "*** Testing key() : usage variations ***\n";

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// get a class
class classA
{
	var $var1;
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

// unexpected values to be passed to $array_arg argument
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
       array(),

       // string data
/*19*/ "string",
       'string',
       $heredoc,
       
       // object data
/*22*/ new classA(),

       // undefined data
/*23*/ @$undefined_var,

       // unset data
/*24*/ @$unset_var,

       // resource variable
/*25*/ $fp
);

// loop through each element of $inputs to check the behavior of key()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  var_dump( key($input) );
  $iterator++;
};

fclose($fp);
?>
===DONE===