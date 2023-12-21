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

// test invalid arguments : non-resources
echo "-- Testing fflush(): with invalid arguments --\n";
$invalid_args = vec[
  "string",
  10,
  10.5,
  true,
  vec[1,2,3],
  new stdClass
];

/* loop to test fflush() with different invalid type of args */
for($loop_counter = 1; $loop_counter <= count($invalid_args); $loop_counter++) {
  echo "-- Iteration $loop_counter --\n";
  try { var_dump( fflush($invalid_args[$loop_counter - 1]) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
echo "\n*** Done ***";

unlink($filename);
}
