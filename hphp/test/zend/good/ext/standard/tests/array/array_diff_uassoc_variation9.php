<?hh
/* Prototype  : array array_diff_uassoc(array arr1, array arr2 [, array ...], callback key_comp_func)
 * Description: Computes the difference of arrays with additional index check which is performed by a
 *                 user supplied callback function
 * Source code: ext/standard/array.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff_uassoc() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
$input_array = dict[10 => 10, 12 => 12];

$input_arrays = dict[
      'decimal indexed' => dict[10 => 10, -17 => -17],
      'octal indexed' => dict[ 012 => 10, -011 => -011,],
      'hexa  indexed' => dict[0xA => 10, -0x7 => -0x7 ],
];

foreach($input_arrays as $key =>$value) {
      echo "\n--$key--\n";
      var_dump( array_diff_uassoc($input_array, $value, ($a, $b) ==> strcasecmp((string)$a, (string)$b)) );
      var_dump( array_diff_uassoc($value, $input_array, ($a, $b) ==> strcasecmp((string)$a, (string)$b)) );
}

echo "===DONE===\n";
}
