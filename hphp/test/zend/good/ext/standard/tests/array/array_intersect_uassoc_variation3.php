<?hh
/* Prototype  : array array_intersect_uassoc(array arr1, array arr2 [, array ...], callback key_compare_func)
 * Description: Computes the intersection of arrays with additional index check, compares indexes by a callback function
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

//Callback function
function key_compare_func($a, $b) {
    if ($a === $b) {
        return 0;
    }
    return ($a > $b) ? 1 : -1;
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_intersect_uassoc() : usage variation ***\n";

// Initialise function arguments
$array1 = darray["a" => "green", "b" => "brown", "c" => "blue", 0 => "red"];
$array2 = darray["a" => "green", 0 => "yellow", 1 => "red"];

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//resource variable
$fp = fopen(__FILE__, "r");

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
      'int -12345' => -12345,

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
      var_dump( array_intersect_uassoc($array1, $array2, $value, fun('key_compare_func')) );
};

fclose($fp);
echo "===DONE===\n";
}
