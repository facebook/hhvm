<?hh
/* Prototype  : int readfile(string filename [, bool use_include_path[, resource context]])
 * Description: Output a file or a URL
 * Source code: ext/standard/file.c
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
echo "*** Testing readfile() : usage variation ***\n";
set_error_handler(test_error_handler<>);

// Initialise function arguments not being substituted (if any)
$filename = sys_get_temp_dir().'/'.'readFileVar5.tmp';
$use_include_path = false;
$h = fopen($filename,'wb');
fwrite($h, "testing readfile");
fclose($h);


// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// add arrays
$index_array = vec[1, 2, 3];
$assoc_array = dict['one' => 1, 'two' => 2];

//array of values to iterate over
$inputs = dict[

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


];

// loop through each element of the array for use_include_path

foreach($inputs as $key =>$value) {
      echo "\n--$key--\n";
            $res = false;
      try { $res = readfile($filename, $value); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
      if ($res == false) {
         echo "File not read\n";
      }
      else {
         echo "\n";
      }
};

unlink($filename);

echo "===DONE===\n";
}
