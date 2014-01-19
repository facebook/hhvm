<?php
ini_set('precision', 14);

/* Prototype  : float floor  ( float $value  )
 * Description: Round fractions down.
 * Source code: ext/standard/math.c
 */

echo "*** Testing floor() : usage variations ***\n";
//get an unset variable
$unset_var = 10;
unset ($unset_var);

// get a class
class classA
{
}

// heredoc string
$heredoc = <<<EOT
abc
xyz
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to $value argument
$inputs = array(
       // null data
/* 1*/ NULL,
       null,

       // boolean data
/* 3*/ true,
       false,
       TRUE,
       FALSE,
       
       // empty data
/* 7*/ "",
       '',
       array(),

       // string data
/*10*/ "abcxyz",
       'abcxyz}',
       $heredoc,
       
       // object data
/*13*/ new classA(),

       // undefined data
/*14*/ @$undefined_var,

       // unset data
/*15*/ @$unset_var,

       // resource variable
/*16*/ $fp
);

// loop through each element of $inputs to check the behaviour of floor()
$iterator = 1;
foreach($inputs as $input) {
	echo "\n-- Iteration $iterator --\n";
	var_dump(floor($input));
	$iterator++;
};
fclose($fp);
?>
===Done===