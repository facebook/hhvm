<?hh
/* Prototype  : array array_diff_ukey(array arr1, array arr2 [, array ...], callback key_comp_func)
 * Description: Returns the entries of arr1 that have keys which are not present in any of the others arguments.
 * Source code: ext/standard/array.c
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

function key_compare_func($key1, $key2)
{
    if ($key1 == $key2) {
        return 0;
    }
    return ($key1 > $key2)? 1:-1;
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff_ukey() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
$array2 = darray['green' => 5, 'blue' => 6, 'yellow' => 7, 'cyan'   => 8];
$array3 = darray['blue'  => 1, 'red'  => 2, 'green'  => 3, 'purple' => 4];

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//resource variable
$fp = fopen(__FILE__, "r");

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

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

      // resource data
      'resource' => $fp,
];

// loop through each element of the array for arr1
foreach($inputs as $key =>$value) {
      echo "\n--$key--\n";
      var_dump( array_diff_ukey($value, $array2, fun('key_compare_func')) );
      var_dump( array_diff_ukey($value, $array2, $array3, fun('key_compare_func')) );
};

fclose($fp);
echo "===DONE===\n";
}
