<?hh
/* Prototype  : array date_sun_info ( int $time , float $latitude , float $longitude )
 * Description:  Returns an array with information about sunset/sunrise and twilight begin/end.
 * Source code: ext/standard/data/php_date.c
 */
<<__EntryPoint>> function main(): void {
date_default_timezone_set("UTC");

echo "*** Testing date_sun_info() : usage variations ***\n";

$inputs = vec[
       // int data
/*1*/  0,
       1,
       12345,
       -2345,
];

// loop through each element of $inputs to check the behaviour of date_sun_info()
$iterator = 1;
foreach($inputs as $input) {
    echo "\n-- Iteration $iterator --\n";
    try { var_dump(date_sun_info($input,  31.7667, 35.2333)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
    $iterator++;
};
echo "===Done===";
}
