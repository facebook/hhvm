<?php
/* Prototype  : string uniqid  ([ string $prefix= ""  [, bool $more_entropy= false  ]] )
 * Description: Gets a prefixed unique identifier based on the current time in microseconds. 
 * Source code: ext/standard/uniqid.c
*/
echo "*** Testing uniqid() : error conditions ***\n";

echo "\n-- Testing uniqid() function with more than expected no. of arguments --\n";
$prefix = null;
$more_entropy = false;
$extra_arg = false;
var_dump(uniqid($prefix, $more_entropy, $extra_arg));

echo "\n-- Testing uniqid() function with invalid values for \$prefix --\n";
class class1{}
$obj = new class1();
$res = fopen(__FILE__, "r"); 
$array = array(1,2,3);

uniqid($array, false);
uniqid($res, false);
uniqid($obj, false);

fclose($res);

?>
===DONE===