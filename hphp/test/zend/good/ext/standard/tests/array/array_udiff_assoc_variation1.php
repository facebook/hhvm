<?hh
/* Prototype  : array array_udiff_assoc(array arr1, array arr2 [, array ...], callback key_comp_func)
 * Description: Returns the entries of arr1 that have values which are not present in any of the others arguments but do additional checks whether the keys are equal. Keys are compared by user supplied function.
 * Source code: ext/standard/array.c
 * Alias to functions:
 */

// define some classes
class classWithToString
{
    public function __toString() {
        return "Class A object";
    }
}

class classWithoutToString
{
}
<<__EntryPoint>> function main(): void {
include('compare_function.inc');
echo "*** Testing array_udiff_assoc() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
$arr2 = varray[1, 2];

$key_comp_func = 'compare_function';

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// add arrays
$index_array = varray [1, 2, 3];
$assoc_array = darray ['one' => 1, 'two' => 2];

//array of values to iterate over
$inputs = darray[

      // int data
      'int 0' => 0,
      'int 1' => 1,
      'int 12345' => 12345,
      'int -12345' => -2345,

      // float data
      'float 10.5' => 10.5,
      'float -10.5' => -10.5,
      'float 12.3456789000e10' => 12.3456789000e10,
      'float -12.3456789000e10' => -12.3456789000e10,
      'float .5' => .5,

      // null data
      'uppercase NULL' => NULL,
      'lowercase null' => null,

      // boolean data
      'lowercase true' => true,
      'lowercase false' =>false,
      'uppercase TRUE' =>TRUE,
      'uppercase FALSE' =>FALSE,

      // empty data
      'empty string DQ' => "",
      'empty string SQ' => '',

      // string data
      'string DQ' => "string",
      'string SQ' => 'string',
      'mixed case string' => "sTrInG",
      'heredoc' => $heredoc,

      // object data
      'instance of classWithToString' => new classWithToString(),
      'instance of classWithoutToString' => new classWithoutToString(),

      // undefined data
      'undefined var' => @$undefined_var,

      // unset data
      'unset var' => @$unset_var,
];

// loop through each element of the array for arr1

foreach($inputs as $key =>$value) {
      echo "\n--$key--\n";
      var_dump( array_udiff_assoc($value, $arr2, $key_comp_func) );
};

echo "===DONE===\n";
}
