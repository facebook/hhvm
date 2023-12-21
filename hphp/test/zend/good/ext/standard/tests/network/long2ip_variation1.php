<?hh
/* Prototype  : string long2ip(int proper_address)
 * Description: Converts an (IPv4) Internet network address into a string in Internet standard dotted format
 * Source code: ext/standard/basic_functions.c
 * Alias to functions:
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

// Define error handler
function test_error_handler($err_no, $err_msg, $filename, $linenum, $vars) :mixed{
    if (error_reporting() != 0) {
        // report non-silenced errors
        echo "Error: $err_no - $err_msg, $filename($linenum)\n";
    }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing long2ip() : usage variation ***\n";
set_error_handler(test_error_handler<>);

// Initialise function arguments not being substituted (if any)

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// add arrays
$index_array = vec[1, 2, 3];
$assoc_array = dict['one' => 1, 'two' => 2];

// resource
$res = fopen(__FILE__,'r');

//array of values to iterate over
$inputs = dict[

      // float data
      'float 10.5' => 10.5,
      'float -10.5' => -10.5,
      'float .5' => .5,

      // array data
      'empty array' => vec[],
      'int indexed array' => $index_array,
      'associative array' => $assoc_array,
      'nested arrays' => vec['foo', $index_array, $assoc_array],

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

      // resource
      'resource' => $res,
];

// loop through each element of the array for proper_address

foreach($inputs as $key =>$value) {
      echo "\n--$key--\n";
      try { var_dump( long2ip($value) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
};

fclose($res);

echo "===DONE===\n";
}
