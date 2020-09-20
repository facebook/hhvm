<?hh
/* Prototype  : int fwrite(resource fp, string str [, int length])
 * Description: Binary-safe file write
 * Source code: ext/standard/file.c
 * Alias to functions: bzwrite fputs gzwrite
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

// Define error handler
function test_error_handler($err_no, $err_msg, $filename, $linenum, $vars) {
    if (error_reporting() != 0) {
        // report non-silenced errors
        echo "Error: $err_no - $err_msg, $filename($linenum)\n";
    }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing fwrite() : usage variation ***\n";
set_error_handler(fun('test_error_handler'));

// Initialise function arguments not being substituted (if any)

$filename = __SystemLib\hphp_test_tmppath('fwriteVar5.tmp');



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

      // array data
      'empty array' => varray[],
      'int indexed array' => $index_array,
      'associative array' => $assoc_array,
      'nested arrays' => varray['foo', $index_array, $assoc_array],

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

      // object data
      'instance of classWithToString' => new classWithToString(),
      'instance of classWithoutToString' => new classWithoutToString(),

      // undefined data
      'undefined var' => @$undefined_var,

      // unset data
      'unset var' => @$unset_var,
];

// loop through each element of the array for str

foreach($inputs as $key =>$value) {
      echo "\n--$key--\n";
      $fp = fopen($filename,'w');
      try { fwrite($fp, $value); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
      fclose($fp);
      readfile($filename);
};
unlink($filename);

echo "===DONE===\n";
}
