<?hh
/*
 Prototype: int fputcsv ( resource $handle [, array $fields [, string $delimiter [, string $enclosure]]] );
 Description:fputcsv() formats a line (passed as a fields array) as CSV and write it to the specified file
   handle. Returns the length of the written string, or FALSE on failure.
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing error conditions ***\n";
// zero argument
echo "-- Testing fputcsv() with zero argument --\n";
try { var_dump( fputcsv() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// more than expected no. of args
echo "-- Testing fputcsv() with more than expected number of arguments --\n";
$fp = fopen(__FILE__, "r");
$fields = vec["fld1", "fld2"];
$delim = ";";
$enclosure ="\"";
try { var_dump( fputcsv($fp, $fields, $delim, $enclosure, $fp) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
fclose($fp);

// test invalid arguments : non-resources
echo "-- Testing fputcsv() with invalid arguments --\n";
$invalid_args = vec[
  "string",
  10,
  10.5,
  true,
  vec[1,2,3],
  new stdClass,
];
/* loop to test fputcsv() with different invalid type of args */
for($loop_counter = 1; $loop_counter <= count($invalid_args); $loop_counter++) {
  echo "-- Iteration $loop_counter --\n";
  try { var_dump( fputcsv($invalid_args[$loop_counter - 1]) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // with default args
  try { var_dump( fputcsv($invalid_args[$loop_counter - 1], $fields, $delim, $enclosure) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // all args specified
}

echo "Done\n";
}
