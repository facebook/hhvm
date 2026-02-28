<?hh
/* Prototype  : int ip2long(string ip_address)
 * Description: Converts a string containing an (IPv4) Internet Protocol dotted address into a proper address
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
echo "*** Testing ip2long() : usage variation ***\n";
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
      // empty data
      'empty string DQ' => "",
      'empty string SQ' => '',
];

// loop through each element of the array for ip_address

foreach($inputs as $key =>$value) {
      echo "\n--$key--\n";
      try { var_dump( ip2long($value) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
};

fclose($res);

echo "===DONE===\n";
}
