<?hh
/* Prototype  : int fwrite(resource fp, string str [, int length])
 * Description: Binary-safe file write
 * Source code: ext/standard/file.c
 * Alias to functions: bzwrite fputs gzwrite
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
echo "*** Testing fwrite() : usage variation ***\n";
set_error_handler(test_error_handler<>);

// Initialise function arguments not being substituted (if any)

$filename = sys_get_temp_dir().'/'.'fwriteVar5.tmp';




// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// add arrays
$index_array = vec[1, 2, 3];
$assoc_array = dict['one' => 1, 'two' => 2];

//array of values to iterate over
$inputs = dict[
      // empty data
      'empty string DQ' => "",
      'empty string SQ' => '',
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
