<?php
ini_set('precision ',  14);

/* 
 * proto float tan(float number)
 * Function is implemented in ext/standard/math.c
*/ 

$file_path = dirname(__FILE__);
require($file_path."/allowed_rounding_error.inc");

$sixty = M_PI / 3.0;
$thirty = M_PI / 6.0;

echo "tan 60 = ";
var_dump(tan($sixty));
if (allowed_rounding_error(tan($sixty),1.7320508075689)) {
	echo "Pass\n";
}
else {
	echo "Fail\n";
}
 
echo "tan 30 = ";
var_dump(tan($thirty));
if (allowed_rounding_error(tan($thirty),0.57735026918963)) {
	echo "Pass\n";
}
else {
	echo "Fail\n";
}

?>