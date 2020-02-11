<?hh
/* Prototype  : array array_intersect_key(array arr1, array arr2 [, array ...])
 * Description: Returns the entries of arr1 that have keys which are present in all the other arguments. 
 * Source code: ext/standard/array.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_intersect_key() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
$input_array = darray[0 => '0', -1 => '-1' , 02 => 'two', -07 => '-07', 0xA => '0xA', -0xC => '-0xc']; 

$input_arrays = darray[
      'decimal indexed' => darray[10 => '10', '-17' => '-17'],
      'octal indexed' => darray[-011 => '-011', 012 => '012'],
      'hexa  indexed' => darray[0x12 => '0x12', -0x7 => '-0x7', ],
];

foreach($input_arrays as $key =>$value) {
      echo "\n--$key--\n";
      var_dump( array_intersect_key($input_array, $value) );
      var_dump( array_intersect_key($value,$input_array ) );
}
echo "===DONE===\n";
}
