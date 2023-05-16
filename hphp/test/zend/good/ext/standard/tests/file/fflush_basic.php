<?hh
/*  Prototype: bool fflush ( resource $handle );
    Description: Flushes the output to a file
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing fflush(): writing to a file and reading the contents ***\n";
$data = <<<EOD
first line of string
second line of string
third line of string
EOD;

$filename = sys_get_temp_dir().'/'.'fflush_basic.tmp';

// opening a file
$file_handle = fopen($filename, "w");
if($file_handle == false)
  exit("Error:failed to open file $filename");

if(substr(PHP_OS, 0, 3) == "WIN")  {
    $data = str_replace("\r",'', $data);
}

// writing data to the file
var_dump( fwrite($file_handle, $data) );
var_dump( fflush($file_handle) );
var_dump( readfile($filename) );

echo "\n*** Testing fflush(): for return type ***\n";
$return_value = fflush($file_handle);
var_dump( is_bool($return_value) );
fclose($file_handle);
echo "\n*** Done ***";

unlink($filename);
}
