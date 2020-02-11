<?hh
/* Prototype  : array array_intersect_key(array arr1, array arr2 [, array ...])
 * Description: Returns the entries of arr1 that have keys which are present in all the other arguments. 
 * Source code: ext/standard/array.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_intersect_key() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
$input_array = darray[0 => '0', 1 => '1' , -10 => '-10' , null => 'null']; 
//get an unset variable
$unset_var = 10;
unset ($unset_var);

$input_arrays = darray[
      'null indexed' => darray[NULL => 'null 1', null => 'null 2'],
      'undefined indexed' => darray[@$undefined_var => 'undefined'],
      'unset  indexed' => darray[@$unset_var => 'unset'],
];

foreach($input_arrays as $key =>$value) {
      echo "\n--$key--\n";
      var_dump( array_intersect_key($input_array, $value) );
      var_dump( array_intersect_key($value,$input_array ) );
}      
echo "===DONE===\n";
}
