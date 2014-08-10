<?php
/* Prototype  : number abs  ( mixed $number  )
 * Description: Returns the absolute value of number.
 * Source code: ext/standard/math.c
 */

/*
 * Pass different data types as $number argument to abs() to test behaviour
 */

echo "*** Testing abs() : usage variations ***\n";

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// get a class
class classA
{
  public function __toString() {
    return "abs";
  }
}

// heredoc string
$heredoc = <<<EOT
abs
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to $number argument
$inputs = array(

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
/*19*/ "abs",
       'abs',
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

// loop through each element of $inputs to check the behavior of abs()
$iterator = 1;
foreach($inputs as $input) {
	echo "\n-- Iteration $iterator --\n";
	var_dump(abs($input) );
	$iterator++;
};

fclose($fp);
?>
===Done===
