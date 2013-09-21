<?php
/* Prototype  : string vprintf(string $format , array $args)
 * Description: Output a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

echo "*** Testing vprintf() : basic functionality - using octal format ***\n";

// Initialise all required variables
$format = "format";
$format1 = "%o";
$format2 = "%o %o";
$format3 = "%o %o %o";
$arg1 = array(021);
$arg2 = array(021,0347);
$arg3 = array(021,0347,05678);

$result = vprintf($format1,$arg1);
echo "\n";
var_dump($result); 

$result = vprintf($format2,$arg2);
echo "\n";
var_dump($result); 

$result = vprintf($format3,$arg3);
echo "\n";
var_dump($result); 

?>
===DONE===