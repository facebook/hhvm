<?hh
/* Prototype  : array array_intersect_ukey(array arr1, array arr2 [, array ...], callback key_compare_func)
 * Description: Computes the intersection of arrays using a callback function on the keys for comparison.
 * Source code: ext/standard/array.c
 */

// define some classes
class classWithToString
{
    public function __toString() :mixed{
        return "Class A object";
    }
}

class classWithoutToString
{
}

//Call back function
function key_compare_func($key1, $key2)
:mixed{
    if ($key1 == $key2)
        return 0;
    else
        return ($key1 > $key2)? 1:-1;
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_intersect_ukey() : usage variation ***\n";

//Initialise arguments
$array2 = dict['green' => 5, 'blue' => 6, 'yellow' => 7, 'cyan'   => 8];
$array3 = dict['green' => 5, 'cyan'   => 8];


//resource variable
$fp = fopen(__FILE__, "r");

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

//array of values to iterate over
$inputs = dict[

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



      // resource data
      'resource var' => $fp,
];

// loop through each element of the array for arr1

foreach($inputs as $key =>$value) {
      echo "\n--$key--\n";
      var_dump( array_intersect_ukey($value, $array2, key_compare_func<>) );
      var_dump( array_intersect_ukey($value, $array2, $array3, key_compare_func<>) );
};

fclose($fp);
echo "===DONE===\n";
}
