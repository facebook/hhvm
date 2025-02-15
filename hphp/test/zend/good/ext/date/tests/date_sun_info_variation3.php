<?hh
/* Prototype  : array date_sun_info ( int $time , float $latitude , float $longitude )
 * Description:  Returns an array with information about sunset/sunrise and twilight begin/end.
 * Source code: ext/standard/data/php_date.c
 */
<<__EntryPoint>> function main(): void {
date_default_timezone_set("UTC");

echo "*** Testing date_sun_info() : usage variations ***\n";

$inputs = vec[
       // float data
/*5*/  10.5,
       -10.5,
       12.3456789000e10,
       12.3456789000E-10,
       .5,
];

// loop through each element of $inputs to check the behaviour of date_sun_info()
$iterator = 1;

foreach($inputs as $input) {
    echo "\n-- Iteration $iterator --\n";
    try { var_dump(date_sun_info(strtotime("2006-12-12"), 31.7667, $input)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
    $iterator++;
};
echo "===Done===";
}
