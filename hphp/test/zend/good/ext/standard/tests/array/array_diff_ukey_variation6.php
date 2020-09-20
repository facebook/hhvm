<?hh
/* Prototype  : array array_diff_ukey(array arr1, array arr2 [, array ...], callback key_comp_func)
 * Description: Returns the entries of arr1 that have keys which are not present in any of the others arguments.
 * Source code: ext/standard/array.c
 */

function key_compare_func($key1, $key2)
{
  return strcasecmp((string)$key1, (string)$key2);
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff_ukey() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
$input_array = darray[-07 => '-07', 0xA => '0xA'];

$input_arrays = darray[
      'decimal indexed' => darray[10 => '10', '-17' => '-17'],
      'octal indexed' => darray[-011 => '-011', 012 => '012'],
      'hexa  indexed' => darray[0x12 => '0x12', -0x7 => '-0x7', ],
];

foreach($input_arrays as $key =>$value) {
      echo "\n--$key--\n";
      var_dump( array_diff_ukey($value, $input_array, fun('key_compare_func')) );
      var_dump( array_diff_ukey($input_array, $value, fun('key_compare_func')) );
}
echo "===DONE===\n";
}
