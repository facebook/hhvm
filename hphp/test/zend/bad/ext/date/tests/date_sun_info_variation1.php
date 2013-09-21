<?php
/* Prototype  : array date_sun_info ( int $time , float $latitude , float $longitude )
 * Description:  Returns an array with information about sunset/sunrise and twilight begin/end.
 * Source code: ext/standard/data/php_date.c
 */
 
date_default_timezone_set("UTC");

echo "*** Testing date_sun_info() : usage variations ***\n";

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// heredoc string
$heredoc = <<<EOT
abc
xyz
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

$inputs = array(
       // int data
/*1*/  0,
       1,
       12345,
       -2345,

       // float data
/*5*/  10.5,
       -10.5,
       12.3456789000e5,
       12.3456789000E-5,
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
/*19*/ "abcxyz",
       'abcxyz',
       $heredoc,
       
       // undefined data
/*22*/ @$undefined_var,

       // unset data
/*23*/ @$unset_var,

       // resource variable
/*24*/ $fp
);

// loop through each element of $inputs to check the behaviour of date_sun_info()
$iterator = 1;
foreach($inputs as $input) {
    echo "\n-- Iteration $iterator --\n";
    var_dump(date_sun_info($input,  31.7667, 35.2333));
    $iterator++;
};

?>
===Done===