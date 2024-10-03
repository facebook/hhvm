<?hh
/* Prototype  : float exp  ( float $arg  )
 * Description: Returns e raised to the power of arg.
 * Source code: ext/standard/math.c
 */

<<__EntryPoint>> function main(): void {
echo "*** Testing exp() : usage variations ***\n";

// unexpected values to be passed to $arg argument
$inputs = vec[
       // float data
/*6*/  10.5,
       -10.5,
       12.3456789000e10,
       12.3456789000E-10,
       .5,
];

// loop through each element of $inputs to check the behaviour of exp()
$iterator = 1;
foreach($inputs as $input) {
    echo "\n-- Iteration $iterator --\n";
    try { var_dump(exp($input)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
    $iterator++;
};
echo "===Done===";
}
