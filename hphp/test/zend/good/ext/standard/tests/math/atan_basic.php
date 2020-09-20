<?hh
/*
 * proto float atan(float number)
 * Function is implemented in ext/standard/math.c
 */

<<__EntryPoint>> function main(): void {
require(__DIR__."/allowed_rounding_error.inc");

echo "atan 1.7320508075689 = ";
$atan1 = 360 * atan(1.7320508075689) / (2.0 * M_PI);
var_dump($atan1);
if (allowed_rounding_error($atan1 ,60 )) {
    echo "Pass\n";
}
else {
    echo "Fail\n";
}

echo "atan 0.57735026918963 = ";
$atan2 = 360 * atan(0.57735026918963) / (2.0 * M_PI);
var_dump($atan2);
if (allowed_rounding_error($atan2 ,30 )) {
    echo "Pass\n";
}
else {
    echo "Fail\n";
}
}
