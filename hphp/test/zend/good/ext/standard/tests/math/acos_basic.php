<?php
/* 
 * proto float acos(float number)
 * Function is implemented in ext/standard/math.c
*/ 

$file_path = dirname(__FILE__);
require($file_path."/allowed_rounding_error.inc");


//output is in degrees

echo "acos .5  = ";
$acosv1 = 360.0  * acos(0.5) / (2.0 * M_PI );
var_dump($acosv1);
if (allowed_rounding_error($acosv1 ,60 )) {
	echo "Pass\n";
}
else {
	echo "Fail\n";
}

echo "acos 0.86602540378444 = ";
$acosv2 = 360.0  * acos(0.86602540378444) / (2.0 * M_PI );
var_dump($acosv2);
if (allowed_rounding_error($acosv2 ,30 )) {
	echo "Pass\n";
}
else {
	echo "Fail\n";
}


echo "acos 1.0 = ";
$acosv3 = 360.0  * acos(1.0) / (2.0 * M_PI);
var_dump($acosv3);
if (allowed_rounding_error($acosv3 ,0 )) {
	echo "Pass\n";
}
else {
	echo "Fail\n";
}


echo "acos 0.0 = ";
$acosv4 = 360.0  * acos(0.0) / (2.0 * M_PI );
var_dump($acosv4);
if (allowed_rounding_error($acosv3 ,0 )) {
	echo "Pass\n";
}
else {
	echo "Fail\n";
}

?>