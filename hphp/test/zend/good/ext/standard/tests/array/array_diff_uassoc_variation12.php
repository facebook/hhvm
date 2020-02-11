<?hh
/* Prototype  : array array_diff_uassoc(array arr1, array arr2 [, array ...], callback key_comp_func)
 * Description: Computes the difference of arrays with additional index check which is performed by a
 *                 user supplied callback function
 * Source code: ext/standard/array.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff_uassoc() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
$input_array = darray[10 => '10', "" => ''];

//get an unset variable
$unset_var = 10;
unset ($unset_var);

$input_arrays = darray[
      'null indexed' => darray[NULL => NULL, null => null],
      'undefined indexed' => darray[@$undefined_var => @$undefined_var],
      'unset indexed' => darray[@$unset_var => @$unset_var],
];

foreach($input_arrays as $key =>$value) {
      echo "\n--$key--\n";
      var_dump( array_diff_uassoc($input_array, $value, ($a, $b) ==> strcasecmp((string)$a, (string)$b)) );
      var_dump( array_diff_uassoc($value, $input_array, ($a, $b) ==> strcasecmp((string)$a, (string)$b)) );
}

echo "===DONE===\n";
}
