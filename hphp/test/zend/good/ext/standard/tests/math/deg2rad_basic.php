<?hh
/*
 * proto float deg2rad(float number)
 * Function is implemented in ext/standard/math.c
 */

<<__EntryPoint>> function main(): void {
require(__DIR__."/allowed_rounding_error.inc");

$arg_0 = 0.0;
$arg_1 = 90.0;
$arg_2 = 180.0;
$arg_3 = 360.0;


$arg_0__str = (string)($arg_0);


echo "deg2rad $arg_0__str = ";
$r0 = deg2rad($arg_0);
var_dump($r0);
if (allowed_rounding_error($r0 ,0 )) {
    echo "Pass\n";
}
else {
    echo "Fail\n";
}

$arg_1__str = (string)($arg_1);

echo "deg2rad $arg_1__str = ";
$r1 = deg2rad($arg_1);
var_dump($r1);
if (allowed_rounding_error($r1 ,1.5707963267949 )) {
    echo "Pass\n";
}
else {
    echo "Fail\n";
}
$arg_2__str = (string)($arg_2);
echo "deg2rad $arg_2__str = ";
$r2 = deg2rad($arg_2);
var_dump($r2);
if (allowed_rounding_error($r2 ,3.1415926535898 )) {
    echo "Pass\n";
}
else {
    echo "Fail\n";
}
$arg_3__str = (string)($arg_3);
echo "deg2rad $arg_3__str = ";
$r3 = deg2rad($arg_3);
var_dump($r3);
if (allowed_rounding_error($r3 ,6.2831853071796 )) {
    echo "Pass\n";
}
else {
    echo "Fail\n";
}
}
