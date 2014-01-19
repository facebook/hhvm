<?php
/* Prototype  : string base_convert  ( string $number  , int $frombase  , int $tobase  )
 * Description: Convert a number between arbitrary bases.
 * Source code: ext/standard/math.c
 */

echo "*** Testing base_convert() : usage variations ***\n";

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// heredoc string
$heredoc = <<<EOT
abc
xyz
EOT;

$inputs = array(
       // int data
/*1*/  0,
       1,
       -1,
       -12,       
       2147483647,

       // float data
/*6*/  10.5,
       -10.5,
       1.234567e2,
       1.234567E-2,
       .5,

       // null data
/*11*/ NULL,
       null,

       // boolean data
/*13*/ true,
       false,
       TRUE,
       FALSE,
       
       // empty data
/*17*/ "",
       '',
       array(),

       // string data
/*20*/ "abcxyz",
       'abcxyz',
       $heredoc,
       
       // undefined data
/*23*/ @$undefined_var,

       // unset data
/*24*/ @$unset_var,
);

// loop through each element of $inputs to check the behaviour of base_convert()
$iterator = 1;
foreach($inputs as $input) {
	echo "\n-- Iteration $iterator --\n";
	var_dump(base_convert(25, $input, 8));
	$iterator++;
};
?>
===Done===