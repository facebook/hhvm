<?hh

/* Prototype: int fseek ( resource $handle, int $offset [, int $whence] );
   Description: Seeks on a file pointer

   Prototype: bool rewind ( resource $handle );
   Description: Rewind the position of a file pointer

   Prototype: int ftell ( resource $handle );
   Description: Tells file pointer read/write position
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing fseek() : error conditions ***\n";
// zero argument
echo "-- Testing fseek() with zero argument --\n";
try { var_dump( fseek() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// unexpected no. of args
echo "-- Testing fseek() with unexpected number of arguments --\n";
$fp = fopen(__FILE__, "r");
try { var_dump( fseek($fp) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( fseek($fp, 10, $fp,10) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// test invalid arguments : non-resources
echo "-- Testing fseek() with invalid arguments --\n";
$invalid_args = vec[
  "string",
  10,
  10.5,
  true,
  vec[1,2,3],
  new stdClass
];
/* loop to test fseek() with different invalid type of args */
for($loop_counter = 1; $loop_counter <= count($invalid_args); $loop_counter++) {
  echo "-- Iteration $loop_counter --\n";
  try { var_dump( fseek($invalid_args[$loop_counter - 1], 10) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

// fseek() on a file handle which is already closed
echo "-- Testing fseek() with closed/unset file handle --";
fclose($fp);
var_dump(fseek($fp,10));

// fseek() on a file handle which is unset
$file_handle = fopen(__FILE__, "r");
unset($file_handle); //unset file handle
try { var_dump( fseek($file_handle,10)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}
