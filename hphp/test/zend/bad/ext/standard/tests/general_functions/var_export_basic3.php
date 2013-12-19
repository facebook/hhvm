<?php
ini_set('serialize_precision', 17);

/* Prototype  : mixed var_export(mixed var [, bool return])
 * Description: Outputs or returns a string representation of a variable 
 * Source code: ext/standard/var.c
 * Alias to functions: 
 */

echo "*** Testing var_export() with valid float values ***\n";
// different valid  float vlaues 
$valid_floats = array(
	  "-2147483649" => -2147483649, // float value
	  "2147483648" => 2147483648,  // float value
	  "-0x80000001" => -0x80000001, // float value, beyond max negative int
	  "0x800000001" => 0x800000001, // float value, beyond max positive int
	  "020000000001" => 020000000001, // float value, beyond max positive int
	  "-020000000001" => -020000000001, // float value, beyond max negative int
	  "0.0" => 0.0,
	  "-0.1" => -0.1,
	  "10.0000000000000000005" => 10.0000000000000000005,
	  "10.5e+5" => 10.5e+5,
	  "1e5" => 1e5,
	  "1e-5" => 1e-5,
	  "1e+5" => 1e+5,
	  "1E5" => 1E5,
	  "1E+5" => 1E+5,
	  "1E-5" => 1E-5,
	  ".5e+7" => .5e+7,
	  ".6e-19" => .6e-19,
	  ".05E+44" => .05E+44,
	  ".0034E-30" => .0034E-30
);
/* Loop to check for above float values with var_export() */
echo "\n*** Output for float values ***\n";
foreach($valid_floats as $key => $float_value) {
	echo "\n-- Iteration: $key --\n";
	var_export( $float_value );
	echo "\n";
	var_export( $float_value, FALSE);
	echo "\n";
	var_dump( var_export( $float_value, TRUE) );
	echo "\n";
}

?>
===DONE===