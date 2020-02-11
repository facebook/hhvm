<?hh
/* Prototype  : array array_diff_ukey(array arr1, array arr2 [, array ...], callback key_comp_func)
 * Description: Returns the entries of arr1 that have keys which are not present in any of the others arguments.
 * Source code: ext/standard/array.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff_ukey() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
$input_array = darray[10 => '10', "" => 'empty'];

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
      var_dump( array_diff_ukey($value, $input_array, ($a, $b) ==> strcasecmp((string)$a, (string)$b)) );
      var_dump( array_diff_ukey($input_array, $value, ($a, $b) ==> strcasecmp((string)$a, (string)$b)) );
}

echo "===DONE===\n";
}
