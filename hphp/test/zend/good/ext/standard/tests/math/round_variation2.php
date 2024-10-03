<?hh
/* Prototype  : float round  ( float $val  [, int $precision  ] )
 * Description: Returns the rounded value of val  to specified precision (number of digits
 * after the decimal point)
 * Source code: ext/standard/math.c
 */

// get a class
class classA
{
}
<<__EntryPoint>> function main(): void {
echo "*** Testing round() : usage variations ***\n";


$inputs = vec[
       // int data
/*1*/  0,
       1,
       12345,
       -2345,
       2147483647,
];

// loop through each element of $inputs to check the behaviour of round()
$iterator = 1;
foreach($inputs as $input) {
    echo "\n-- Iteration $iterator --\n";
    try { var_dump(round(123.4456789, $input)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
    $iterator++;
};
echo "===Done===";
}
