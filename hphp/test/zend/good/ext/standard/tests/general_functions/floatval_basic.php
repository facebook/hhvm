<?hh
/* Prototype: float floatval( mixed $var );
 * Description: Returns the float value of var.
 */

// different valid  float values
<<__EntryPoint>> function main(): void {
$valid_floats = dict[
       "0.0"  => 0.0,
       "1.0"  => 1.0,
       "-1.0" => -1.0,
       "1.234" => 1.234,
        "-1.234" => -1.234,
       "1.2e3" => 1.2e3,
       "-1.2e3" => -1.2e3,
       "10.0000000000000000005" => 10.0000000000000000005,
       "10.5e+5" => 10.5e+5,
       "1e5" => 1e5,
       "-1e5" => -1e5,
       "1e5" => 1e-5,
       "-1e-1" => -1e-1,
       "1e+5" => 1e+5,
       "-1e+5" =>-1e+5,
       "1E5" => 1E5,
       "-1E5" => -1E5,
       "1E+5" => 1E+5,
       "-1E5" => -1E+5,
       ".5e+7" => .5e+7,
       "-.5e+7" =>-.5e+7
];

/* loop to check that floatval() recognizes different
   float values, expected output:float value for valid floating point number */
echo "*** Testing floatval() with valid float values ***\n";
foreach ($valid_floats as $key => $value ) {
   echo "\n-- Iteration : $key -- \n";
   var_dump( floatval($value) );
}

/* loop to check that doubleval() also recognizes different
   float values, expected output:float value for valid floating point number */
echo "\n*** Testing doubleval() with valid float values ***\n";
foreach ($valid_floats as $key => $value ) {
   echo "\n-- Iteration : $key -- \n";
   var_dump( doubleval($value) );
}

echo "===DONE===\n";
}
