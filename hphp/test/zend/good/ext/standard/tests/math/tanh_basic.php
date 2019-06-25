<?hh
/*
 * proto float tanh(float number)
 * Function is implemented in ext/standard/math.c
 */

require(__DIR__."/allowed_rounding_error.inc");

<<__EntryPoint>> function main(): void {

echo "tanh .5  = ";
var_dump(tanh(0.5));
if (allowed_rounding_error(tanh(0.5), 0.46211715726001)) {
    echo "Pass\n";
}
else {
    echo "Fail\n";
}

echo "tanh -0.5  = ";
var_dump(tanh(-0.5));
if (allowed_rounding_error(tanh(-0.5), -0.46211715726001)) {
    echo "Pass\n";
}
else {
    echo "Fail\n";
}

echo "tanh 3  = ";
var_dump(tanh(3.0));
if (allowed_rounding_error(tanh(3.0),0.99505475368673 )) {
    echo "Pass\n";
}
else {
    echo "Fail\n";
}

echo "tanh -3  = ";
var_dump(tanh(-3.0));
if (allowed_rounding_error(tanh(-3.0),-0.99505475368673 )) {
    echo "Pass\n";
}
else {
    echo "Fail\n";
}
}
