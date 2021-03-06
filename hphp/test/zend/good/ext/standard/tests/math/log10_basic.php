<?hh
/*
 * proto float log10(float number)
 * Function is implemented in ext/standard/math.c
 */

<<__EntryPoint>> function main(): void {
require(__DIR__."/allowed_rounding_error.inc");

$arg_0 = 1.0;
$arg_1 = 10.0;
$arg_2 = 100.0;

$arg_0__str = (string)($arg_0);

echo "log10 $arg_0__str = ";
$r0 = log10($arg_0);
var_dump($r0);
if (allowed_rounding_error($r0 ,0.0 )) {
    echo "Pass\n";
}
else {
    echo "Fail\n";
}

$arg_1__str = (string)($arg_1);

echo "log10 $arg_1__str = ";
$r1 = log10($arg_1);
var_dump($r1);
if (allowed_rounding_error($r1 ,1.0 )) {
    echo "Pass\n";
}
else {
    echo "Fail\n";
}

$arg_2__str = (string)($arg_2);

echo "log10 $arg_2__str = ";
$r2 = log10($arg_2);
var_dump($r2);
if (allowed_rounding_error($r2 ,2.0 )) {
    echo "Pass\n";
}
else {
    echo "Fail\n";
}
}
