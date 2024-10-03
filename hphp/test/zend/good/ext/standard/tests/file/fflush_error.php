<?hh
/*
 Prototype: bool fflush ( resource $handle );
 Description: Flushes the output to a file
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing error conditions ***\n";


// zero argument
echo "-- Testing fflush(): with zero argument --\n";
try { var_dump( fflush() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// more than expected no. of args
echo "-- Testing fflush(): with more than expected number of arguments --\n";

$filename = sys_get_temp_dir().'/'.'fflush_error.tmp';
$file_handle = fopen($filename, "w");
if($file_handle == false)
  exit("Error:failed to open file $filename");

try { var_dump( fflush($file_handle, $file_handle) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
fclose($file_handle);

echo "\n*** Done ***";

unlink($filename);
}
