<?php

print "- empty ---\n";

$str = "test0123";

var_dump(empty($str[-1]));
var_dump(empty($str[0]));
var_dump(empty($str[1]));
var_dump(empty($str[4])); // 0
var_dump(empty($str[5])); // 1
var_dump(empty($str[8]));
var_dump(empty($str[10000]));
// non-numeric offsets
print "- string ---\n";
var_dump(empty($str['-1']));
var_dump(empty($str['0']));
var_dump(empty($str['1']));
var_dump(empty($str['4'])); // 0
var_dump(empty($str['1.5']));
var_dump(empty($str['good']));
var_dump(empty($str['3 and a half']));
print "- bool ---\n";
var_dump(empty($str[true]));
var_dump(empty($str[false]));
var_dump(empty($str[false][true]));
print "- null ---\n";
var_dump(empty($str[null]));
print "- double ---\n";
var_dump(empty($str[-1.1]));
var_dump(empty($str[-0.8]));
var_dump(empty($str[-0.1]));
var_dump(empty($str[0.2]));
var_dump(empty($str[0.9]));
var_dump(empty($str[M_PI]));
var_dump(empty($str[100.5001]));
print "- array ---\n";
var_dump(empty($str[array()]));
var_dump(empty($str[array(1,2,3)]));
print "- object ---\n";
var_dump(empty($str[new stdClass()]));
print "- resource ---\n";
$f = fopen(__FILE__, 'r');
var_dump(empty($str[$f]));
print "done\n";

?>
