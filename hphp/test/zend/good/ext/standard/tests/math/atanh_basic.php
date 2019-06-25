<?hh
/*
 * proto float atanh(float number)
 * Function is implemented in ext/standard/math.c
 */

require(__DIR__."/allowed_rounding_error.inc");

<<__EntryPoint>> function main(): void {

echo "atanh  0.46211715726001 = ";
var_dump(atanh(0.46211715726001));
if (allowed_rounding_error(atanh(0.46211715726001), 0.5))
{
    echo "Pass\n";
}
else {
    echo "Fail\n";
}

echo "atanh  0.99505475368673 = ";
var_dump(atanh(0.99505475368673));
if (allowed_rounding_error(atanh(0.99505475368673), 3.0))
{
    echo "Pass\n";
}
else {
    echo "Fail\n";
}
}
