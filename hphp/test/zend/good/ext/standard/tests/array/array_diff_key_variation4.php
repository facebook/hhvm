<?hh
/* Prototype  : array array_diff_key(array arr1, array arr2 [, array ...])
 * Description: Returns the entries of arr1 that have keys which are not present in any of the others arguments. 
 * Source code: ext/standard/array.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff_key() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
$input_array = dict[-07 => '-07', 0xA => '0xA']; 

$input_arrays = dict[
      'decimal indexed' => dict[10 => '10', '-17' => '-17'],
      'octal indexed' => dict[-011 => '-011', 012 => '012'],
      'hexa  indexed' => dict[0x12 => '0x12', -0x7 => '-0x7', ],
];

// loop through each element of the array for arr1
foreach($input_arrays as $key =>$value) {
      echo "\n--$key--\n";
      var_dump( array_diff_key($input_array, $value) );
      var_dump( array_diff_key($value, $input_array) );
}
echo "===DONE===\n";
}
