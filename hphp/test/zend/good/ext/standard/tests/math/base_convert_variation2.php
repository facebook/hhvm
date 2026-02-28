<?hh
/* Prototype  : string base_convert  ( string $number  , int $frombase  , int $tobase  )
 * Description: Convert a number between arbitrary bases.
 * Source code: ext/standard/math.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing base_convert() : usage variations ***\n";

$inputs = vec[
       // int data
/*1*/  0,
       1,
       -1,
       -12,
       2147483647,
];

// loop through each element of $inputs to check the behaviour of base_convert()
$iterator = 1;
foreach($inputs as $input) {
    echo "\n-- Iteration $iterator --\n";
    try { var_dump(base_convert(25, $input, 8)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
    $iterator++;
};
echo "===Done===";
}
