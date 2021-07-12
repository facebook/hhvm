<?hh
/*
 * proto float rad2deg(float number)
 * Function is implemented in ext/standard/math.c
 */

<<__EntryPoint>> function main(): void {
require(__DIR__."/allowed_rounding_error.inc");

$arg_0 = 0.0;
$arg_1 = 1.570796327;
$arg_2 = 3.141592654;
$arg_3 = 6.283185307;

$arg_0__str = (string)($arg_0);

echo "rad2deg $arg_0__str= ";
$r0 = rad2deg($arg_0);
var_dump($r0);
if (allowed_rounding_error($r0 ,0 )) {
    echo "Pass\n";
}
else {
    echo "Fail\n";
}
$arg_1__str = (string)($arg_1);
echo "rad2deg $arg_1__str = ";
$r1 = rad2deg($arg_1);
var_dump($r1);
if (allowed_rounding_error($r1 ,90.000000011752)) {
    echo "Pass\n";
}
else {
    echo "Fail\n";
}
$arg_2__str = (string)($arg_2);
echo "rad2deg $arg_2__str  = ";
$r2 = rad2deg($arg_2);
var_dump($r2);
if (allowed_rounding_error($r2 ,180.0000000235 )) {
    echo "Pass\n";
}
else {
    echo "Fail\n";
}
$arg_3__str = (string)($arg_3);
echo "rad2deg $arg_3__str = ";
$r3 = rad2deg($arg_3);
var_dump($r3);
if (allowed_rounding_error($r3 ,359.99999998971 )) {
    echo "Pass\n";
}
else {
    echo "Fail\n";
}
}
