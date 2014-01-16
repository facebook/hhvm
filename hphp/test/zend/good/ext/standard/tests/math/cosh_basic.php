<?php
ini_set('precision ',  14);

/* 
 * proto float cosh(float number)
 * Function is implemented in ext/standard/math.c
*/ 

$file_path = dirname(__FILE__);
require($file_path."/allowed_rounding_error.inc");

echo "cosh .5  = ";
var_dump(cosh(0.5));

if (allowed_rounding_error(cosh(0.5),1.1276259652064)){
	echo "Pass\n";
}
else {
	echo "Fail\n";
}

echo "cosh -0.5  = ";
var_dump(cosh(-0.5));
if (allowed_rounding_error(cosh(-0.5),1.1276259652064)){
	echo "Pass\n";
}
else {
	echo "Fail\n";
}

echo "cosh 3  = ";
var_dump(cosh(3.0));
if (allowed_rounding_error(cosh(3.0), 10.067661995778)){
	echo "Pass\n";
}
else {
	echo "Fail\n";
}

echo "cosh -3  = ";
var_dump(cosh(-3.0));
if (allowed_rounding_error(cosh(-3.0), 10.067661995778)){
	echo "Pass\n";
}
else {
	echo "Fail\n";
}

?>