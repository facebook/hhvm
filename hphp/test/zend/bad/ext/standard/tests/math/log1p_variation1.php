<?php
ini_set('precision', 14);

/* Prototype  : float log1p  ( float $arg  )
 * Description: Returns log(1 + number), computed in a way that is accurate even 
 *				when the value of number is close to zero
 * Source code: ext/standard/math.c
 */

echo "*** Testing log1p() : usage variations ***\n";

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// heredoc string
$heredoc = <<<EOT
abc
xyz
EOT;

// get a class
class classA
{
}

// get a resource variable
$fp = fopen(__FILE__, "r");

$inputs = array(
       // int data
/*1*/  0,
       1,
       12345,
       -2345,       
       2147483647,
       -2147483648,

       // float data
/*7*/  10.5,
       -10.5,
       12.3456789E4,
       12.3456789E-4,
       .5,

       // null data
/*12*/ NULL,
       null,

       // boolean data
/*14*/ true,
       false,
       TRUE,
       FALSE,
       
       // empty data
/*18*/ "",
       '',
       array(),

       // string data
/*21*/ "abcxyz",
       'abcxyz',
       $heredoc,
       
       // object data
/*24*/ new classA(),       
       
       // undefined data
/*25*/ @$undefined_var,

       // unset data
/*26*/ @$unset_var,

       // resource variable
/*27*/ $fp
);

// loop through each element of $inputs to check the behaviour of log1p()
$iterator = 1;
foreach($inputs as $input) {
	echo "\n-- Iteration $iterator --\n";
	var_dump(log1p($input));
	$iterator++;
};
fclose($fp);
?>
===Done===