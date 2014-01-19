<?php
ini_set('precision ',  14);

/* 
 * proto float cos(float number)
 * Function is implemented in ext/standard/math.c
*/ 

$file_path = dirname(__FILE__);
require($file_path."/allowed_rounding_error.inc");


// Use known values to test

$sixty = M_PI / 3.0;
$thirty = M_PI / 6.0;
$ninety = M_PI /2.0;
$oneeighty = M_PI;
$twoseventy = M_PI * 1.5;
$threesixty = M_PI * 2.0;


echo "cos 30 = ";
var_dump(cos($thirty));
if (allowed_rounding_error(cos($thirty),0.86602540378444)) {
	echo "Pass\n";
}
else {
	echo "Fail\n";
}

echo "cos 60 = ";
var_dump(cos($sixty));
if (allowed_rounding_error(cos($sixty),0.5)) {
	echo "Pass\n";
}
else {
	echo "Fail\n";
}

echo "cos 90 = ";
var_dump(cos($ninety));
if (allowed_rounding_error(cos($ninety),0.0)) {
	echo "Pass\n";
}
else {
	echo "Fail\n";
}

echo "cos 180 = ";
var_dump(cos($oneeighty));
if (allowed_rounding_error(cos($oneeighty),-1.0)) {
	echo "Pass\n";
}
else {
	echo "Fail\n";
}

echo "cos 270 = ";
var_dump(cos($twoseventy));
if (allowed_rounding_error(cos($twoseventy),0.0)) {
	echo "Pass\n";
}
else {
	echo "Fail\n";
}

echo "cos 360 = ";
var_dump(cos($threesixty));
if (allowed_rounding_error(cos($threesixty),1.0)) {
	echo "Pass\n";
}
else {
	echo "Fail\n";
}
?>