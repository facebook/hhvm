<?hh

/* Prototype  : float expm1  ( float $arg  )
 * Description: Returns exp(number) - 1, computed in a way that is accurate even
 *              when the value of number is close to zero.
 * Source code: ext/standard/math.c
 */

<<__EntryPoint>> function main(): void {
echo "*** Testing expm1() : usage variations ***\n";

// unexpected values to be passed to $arg argument
$inputs = vec[
       // float data
/*5*/  10.5,
       -10.5,
       12.3456789E4,
       12.3456789E-4,
       .5,
];

// loop through each element of $inputs to check the behaviour of expm1()
$iterator = 1;
foreach($inputs as $input) {
    echo "\n-- Iteration $iterator --\n";
    try { var_dump(expm1($input)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
    $iterator++;
};
echo "===Done===";
}
