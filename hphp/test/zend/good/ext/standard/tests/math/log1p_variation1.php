<?hh
/* Prototype  : float log1p  ( float $arg  )
 * Description: Returns log(1 + number), computed in a way that is accurate even
 *                when the value of number is close to zero
 * Source code: ext/standard/math.c
 */

<<__EntryPoint>> function main(): void {
echo "*** Testing log1p() : usage variations ***\n";

$inputs = vec[
       // float data
/*7*/  10.5,
       -10.5,
       12.3456789E4,
       12.3456789E-4,
       .5,
];

// loop through each element of $inputs to check the behaviour of log1p()
$iterator = 1;
foreach($inputs as $input) {
    echo "\n-- Iteration $iterator --\n";
    try { var_dump(log1p($input)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
    $iterator++;
};
echo "===Done===";
}
