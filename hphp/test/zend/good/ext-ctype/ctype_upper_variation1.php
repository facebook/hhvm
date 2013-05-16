<?php
/* Prototype  : bool ctype_upper(mixed $c)
 * Description: Checks for uppercase character(s) 
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass different data types as $c argument to ctype_upper() to test behaviour
 */

echo "*** Testing ctype_upper() : usage variations ***\n";

$orig = setlocale(LC_CTYPE, "C");

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// get a class
class classA
{
	public function __toString() {
		return "HELLO";
	}
}

// heredoc string
$heredoc = <<<EOT
HI
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to $c argument
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
/*19*/ "STRING",
       'STRING',
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

// loop through each element of $inputs to check the behavior of ctype_upper()
$iterator = 1;
foreach($inputs as $input) {
	echo "\n-- Iteration $iterator --\n";
	var_dump( ctype_upper($input) );
	$iterator++;
};

fclose($fp);

setlocale(LC_CTYPE, $orig);
?>
===DONE===