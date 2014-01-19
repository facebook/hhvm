<?php
ini_set('precision ',  14);

/* 
 * proto float sinh(float number)
 * Function is implemented in ext/standard/math.c
*/ 

$file_path = dirname(__FILE__);
require($file_path."/allowed_rounding_error.inc");

echo "sinh .5  = ";
var_dump(sinh(0.5));
if (allowed_rounding_error(sinh(0.5),0.52109530549375)){
	echo "Pass\n";
}
else {
	echo "Fail\n";
}

echo "sinh -0.5  = ";
var_dump(sinh(-0.5));
if (allowed_rounding_error(sinh(-0.5), -0.52109530549375)){
	echo "Pass\n";
}
else {
	echo "Fail\n";
}

echo "sinh 3  = ";
var_dump(sinh(3.0));
if (allowed_rounding_error(sinh(3.0), 10.01787492741)){
	echo "Pass\n";
}
else {
	echo "Fail\n";
}

echo "sinh -3  = ";
var_dump(sinh(-3.0));
if (allowed_rounding_error(sinh(-3.0), -10.01787492741)){
	echo "Pass\n";
}
else {
	echo "Fail\n";
}

?>