<?hh
/*
 * proto float asinh(float number)
 * Function is implemented in ext/standard/math.c
 */

require(__DIR__."/allowed_rounding_error.inc");

<<__EntryPoint>> function main(): void {
echo "asinh  0.52109530549375= ";
var_dump(asinh(0.52109530549375));
if (allowed_rounding_error(asinh(0.52109530549375), 0.5))
{
    echo "Pass\n";
}
else {
    echo "Fail\n";
}

echo "asinh 10.01787492741= ";
var_dump(asinh(10.01787492741));
if (allowed_rounding_error(asinh(10.01787492741), 3.0))
{
    echo "Pass\n";
}
else {
    echo "Fail\n";
}
}
