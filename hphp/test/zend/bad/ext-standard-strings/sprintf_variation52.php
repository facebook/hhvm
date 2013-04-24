<?php
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

echo "*** Testing sprintf() : with  typical format strings ***\n";

// initialising required variables
$tempnum = 12345;
$tempstring = "abcdefghjklmnpqrstuvwxyz";

echo"\n-- Testing for '%%%.2f' as the format parameter --\n";
var_dump(sprintf("%%%.2f", 1.23456789e10));

echo"\n-- Testing for '%%' as the format parameter --\n";
var_dump(sprintf("%%", 1.23456789e10));

echo"\n-- Testing for precision value more than maximum --\n";
var_dump(sprintf("%.988f", 1.23456789e10));

echo"\n-- Testing for invalid width(-15) specifier --\n";
var_dump(sprintf("%030.-15s", $tempstring));

echo"\n-- Testing for '%X' as the format parameter --\n";
var_dump(sprintf("%X", 12));

echo"\n-- Testing for multiple format parameters --\n";
var_dump(sprintf("%d  %s  %d\n", $tempnum, $tempstring, $tempnum));

echo"\n-- Testing for excess of mixed type arguments  --\n";
var_dump(sprintf("%s", $tempstring, $tempstring, $tempstring));

echo "Done";
?>